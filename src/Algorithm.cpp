#include "Algorithm.hpp"

#include <cmath>
#include <cinttypes>
#include <omp.h>
#include <primesieve/iterator.hpp>

#include "ProcessBatch.hpp"
#include "PerformModulus.hpp"

#include "Context.hpp"
#include "ProgressLogger.hpp"
#include "PrimeGapList.hpp"
#include "PrimeGapIterator.hpp"
#include "TermBuffer.hpp"

namespace {

uint64_t estimate_thread_length(bool is_first_interval, double& weight, const mpmp19::Context& ctx);
uint64_t nth_prime_approx(uint64_t n);
void sequence_interval(uint64_t start_number, uint64_t thread_length, mpmp19::Context& ctx);
void sequence_thread(mpmp19::ThreadState& state, uint64_t start_number, uint64_t end_number);
void find_terms(mpmp19::TermBuffer& results, mpmp19::Context& ctx);

}

namespace mpmp19 {

void run_sequence(Config& cfg, const char* progress_filename, const char* term_filename) {
    Context ctx(cfg);
    initialise_output_file(progress_filename);
    initialise_output_file(term_filename);
    log_start_config(ctx.cfg, progress_filename);

    // time variables
    const double start_time = now_seconds();
    double last_time = start_time;
    double sequence_time = 0.0;
    double mod_time = 0.0;

    // tracker variables
    uint64_t billion_count = 0;
    uint64_t start_number = 1;

    TermBuffer results;
    double weight, t;
    uint64_t thread_length = estimate_thread_length(true, weight, ctx);    // first call initialises the weight value

    for (uint64_t i = 0; i < ctx.cfg.num_intervals; i++) {
        // sequencing phase
        t = now_seconds();
        sequence_interval(start_number, thread_length, ctx);
        sequence_time += now_seconds() - t;

        // modulus phase
        t  = now_seconds();
        find_terms(results, ctx);
        mod_time += now_seconds() - t;

        // deal with any terms found for this interval
        results.sort();
        results.output_terms(term_filename);
        ctx.term_count += results.count();
        log_progress(billion_count, ctx.prime_count, start_time, last_time, progress_filename);

        // update values for next interval
        start_number += thread_length * ctx.cfg.num_threads;
        thread_length = estimate_thread_length(false, weight, ctx);
    }

    double total_time = now_seconds() - start_time;
    log_final_stats(ctx.prime_count, ctx.term_count, total_time, sequence_time, mod_time, progress_filename);
}

}   // namespace

namespace {

using namespace mpmp19;

void sequence_interval(uint64_t start_number, uint64_t thread_length, Context& ctx) {
    std::exception_ptr ex = nullptr;

    #pragma omp parallel num_threads(ctx.cfg.num_threads) 
    {
        try {
            uint32_t i = omp_get_thread_num();
            uint64_t local_start = start_number + i * thread_length;
            uint64_t local_end = local_start + thread_length - 1;
            ThreadState& state = ctx.thread_states[i];

            sequence_thread(state, local_start, local_end);
        } catch(...) {
            #pragma omp critical
            { 
                if (!ex) ex = std::current_exception(); 
            }
        }
    }

    if (ex) std::rethrow_exception(ex);

    // Convert each thread's partial ThreadResult into a cumulative Checkpoint.
    // Scan from left to right accumulating running totals for prime count and square sum
    uint192_t running_sum = ctx.square_sum;
    uint64_t running_count = ctx.prime_count;

    for (auto& state : ctx.thread_states) {
        auto& result = state.result;
        auto& cp = state.checkpoint;

        cp.initial_prime = result.initial_prime;
        cp.initial_square_sum = running_sum;            // cumulative at the START of this thread
        cp.initial_prime_count = running_count;         // cumulative at the START of this thread
        cp.thread_prime_count = result.thread_prime_count;

        running_sum = u192_add_u192(running_sum, result.partial_square_sum);
        running_count += result.thread_prime_count;
    }

    ctx.square_sum = running_sum;
    ctx.prime_count = running_count;
}

void sequence_thread(ThreadState& state, uint64_t start_number, uint64_t end_number) {
    auto& result = state.result;
    auto& gaps = state.gaps;
    auto& it = state.it;

    it.jump_to(start_number, UINT64_MAX);
    gaps.reset();

    uint192_t square_sum = u192_zero();
    uint64_t prime_count = 0;
    uint64_t prev_prime = 0;
    bool is_first_batch = true;

    while (true) {
        it.generate_next_primes();  // generate the next batch of primes ~ 2^10

        // store the initial prime in the thread result and initialise prev_prime
        // we can be certain that it.size_ > 0 as primesieve would have thrown an exception otherwise
        if (is_first_batch) {
            prev_prime = it.primes_[0];
            result.initial_prime = it.primes_[0];
            is_first_batch = false;
        }

        if (it.primes_[it.size_ - 1] <= end_number) {
            // all primes in this batch are valid, i.e. <= end_number
            prime_count += process_full_batch(it, gaps, square_sum, prev_prime);
        } else {
            // some primes in this batch are invalid
            prime_count += process_partial_batch(it, gaps, square_sum, prev_prime, end_number);
            break;
        }
    }

    result.partial_square_sum = square_sum;
    result.thread_prime_count = prime_count;
}

void find_terms(TermBuffer& results, Context& ctx) {
    results.reset();
    std::exception_ptr ex = nullptr;
    
    #pragma omp parallel num_threads(ctx.cfg.num_threads)
    {
        try {
            uint32_t i = omp_get_thread_num();
            auto& state = ctx.thread_states[i];
            perform_modulus_operations_thread(state, results);
        } catch(...) {
            #pragma omp critical
            {
                if (!ex) ex = std::current_exception();
            }
        }
    }

    if (ex) std::rethrow_exception(ex);
}

// Aims to estimate the length of interval required for the initial thread to contain ctx.cfg.primes_per_thread primes
// We do this for the initial thread, as subsequent threads are likely to contain less primes, making this an 
// effective way to control how much memory is used for storing prime gaps
// On the first call (is_first_interval) the function uses a prime number theorem approximation
// On subsequent calls it adjusts the estimate proportionally based on how many primes were found in the previous interval
// returns the thread length (range per thread) and not the total interval length
uint64_t estimate_thread_length(bool is_first_interval, double& weight, const Context& ctx) {
    if (is_first_interval) {
        weight = 1.0;
        return nth_prime_approx(ctx.cfg.primes_per_thread);
    }

    const uint64_t target = ctx.cfg.primes_per_thread;
    const uint64_t actual = ctx.thread_states[0].checkpoint.thread_prime_count;

    constexpr double alpha = 1.0;
    const double ratio = (double)target / (double)(actual + 1);
    weight *= std::pow(ratio, alpha);

    const uint64_t pnt_length = nth_prime_approx(ctx.prime_count + target) - nth_prime_approx(ctx.prime_count);
    return (uint64_t)(weight * pnt_length);
}

// uses the approximation discussed in this paper - https://www.researchgate.net/publication/385813259_The_Nth_Prime_Estimates
// to estimate the value of the nth prime
uint64_t nth_prime_approx(uint64_t n) {
    if (n < 10)
        return n;

    const double x = (double)n;
    const double logx = std::log(x);
    const double loglogx = std::log(logx);

    const double prime_approx = x * (logx - 1.0 + std::log(logx + loglogx - 2.0));
    return (uint64_t)prime_approx;
}

}   // namespace