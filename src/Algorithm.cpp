#include "Algorithm.hpp"

#include <cmath>
#include <cinttypes>
#include <omp.h>
#include <primesieve/iterator.hpp>

#include "ProcessBatch.hpp"
#include "PerformModulus.hpp"

#include "BigInt.hpp"
#include "ProgressLogger.hpp"
#include "PrimeGapList.hpp"
#include "PrimeGapIterator.hpp"
#include "TermBuffer.hpp"
#include "mpmp19_error.hpp"
#include "Hints.hpp"

namespace {

/**
 * Used internally by mpmp19_sequence() to calculate the size of the range each thread gets on the next iteration
 * It uses a prime number theorem approximation to obtain a range with roughly the same number of primes in as the 
 * primes_per_thread_interval stored in cfg. It auto adjusts its size based on the number of primes found in the previous
 * interval to keep the number of primes in each interval roughly constant.
 */
uint64_t estimate_thread_length(bool is_first_interval, double& weight, const mpmp19::Context& ctx);
uint64_t nth_prime_approx(uint64_t n);

/**
 * Takes the start number and the thread length as parameters. This assigns a primesieve iterator to each thread and each
 * thread calls sequence_thread() with its own id. 
 */
void sequence_interval(uint64_t start_number, uint64_t thread_length, mpmp19::Context& ctx);

void sequence_thread(mpmp19::ThreadState& state, uint64_t start_number, uint64_t end_number);

void find_terms(mpmp19::TermBuffer& results, mpmp19::Context& ctx);

}   // namespace

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

    uint64_t billion_count = 0;
    uint64_t start_number = 1;
    TermBuffer results;

    double weight;
    uint64_t thread_length = estimate_thread_length(true, weight, ctx);

    for (uint64_t i = 0; i < ctx.cfg.num_intervals; i++) {
        double t;

        t = now_seconds();
        sequence_interval(start_number, thread_length, ctx);
        sequence_time += now_seconds() - t;

        t  = now_seconds();
        find_terms(results, ctx);
        mod_time += now_seconds() - t;

        results.sort();
        results.output_terms(term_filename);
        ctx.term_count += results.count();
        log_progress(billion_count, ctx.prime_count, start_time, last_time, progress_filename);

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
            auto& state = ctx.thread_states[i];

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
    // We scan left to right, accumulating the running total.
    uint192_t running_sum = ctx.square_sum;
    uint64_t running_count = ctx.prime_count;

    for (uint32_t i = 0; i < ctx.cfg.num_threads; i++) {
        auto& result = ctx.thread_states[i].result;
        auto& cp = ctx.thread_states[i].checkpoint;

        cp.initial_prime = result.initial_prime;
        cp.initial_square_sum = running_sum;           // cumulative BEFORE this thread
        cp.initial_prime_count = running_count;         // cumulative BEFORE this thread
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
    uint64_t prev_prime = 0;
    uint64_t prime_count = 0;
    bool is_first_batch = true;

    while (true) {
        // generate the next batch of primes
        it.generate_next_primes();
        
        MPMP19_ASSERT(it.size_ > 0);  // shouldn't happen but be safe

        // on the first batch, handle the initial prime separately:
        // store it in the checkpoint, give it a gap of 0, and start
        // the main loop from index 1
        if (is_first_batch) {
            prev_prime = it.primes_[0];
            result.initial_prime = it.primes_[0];
            is_first_batch = false;
        }

        if (it.primes_[it.size_ - 1] <= end_number) {
            // all primes in this batch are valid i.e. <= end_number
            prime_count += process_full_batch(it, gaps, square_sum, prev_prime);
        } else {
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

/**
 * Estimates the number range [start, start + result] that contains approximately
 * cfg.primes_per_thread_interval primes starting from start_number.
 *
 * On the first call (start_number == 1 or previous_count == 0), uses the prime
 * number theorem approximation: the nth prime is approximately n * ln(n), so
 * a range of length k * ln(p) near prime p contains approximately k primes.
 *
 * On subsequent calls, adjusts the estimate proportionally based on how many
 * primes the previous interval actually contained vs the target. This corrects
 * for the PNT approximation error which grows slowly with n.
 *
 * Returns the thread length (range per thread), not the total interval length.
 * Total interval length = result * cfg.num_threads.
 */

uint64_t estimate_thread_length(bool is_first_interval, double& weight, const Context& ctx) {
    if (is_first_interval) {
        weight = 1.0;
        return nth_prime_approx(ctx.cfg.primes_per_thread_interval);
    }

    const uint64_t target = ctx.cfg.primes_per_thread_interval;
    const uint64_t actual = ctx.thread_states[0].checkpoint.thread_prime_count;

    constexpr double alpha = 1.0;
    const double ratio = (double)target / (double)(actual + 1);
    weight *= std::pow(ratio, alpha);

    const uint64_t pnt_length = nth_prime_approx(ctx.prime_count + target) - nth_prime_approx(ctx.prime_count);
    return (uint64_t)(weight * pnt_length);
}

uint64_t nth_prime_approx(uint64_t n) {
    if (n < 10)
        return n;

    const double x = (double)n;
    const double logx = std::log(x);
    const double loglogx = std::log(logx);

    // https://www.researchgate.net/publication/385813259_The_Nth_Prime_Estimates
    const double prime_approx = x * (logx - 1.0 + std::log(logx + loglogx - 2.0));
    return (uint64_t)prime_approx;
}

}   // namespace