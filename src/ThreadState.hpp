#ifndef THREAD_STATE_HPP
#define THREAD_STATE_HPP

#include <primesieve/iterator.hpp>
#include "BigInt.hpp"
#include "PrimeGapList.hpp"

namespace mpmp19 {

// What sequence_thread() produces: the per-thread results before
// cumulative state is computed.
struct ThreadResult {
    uint64_t  initial_prime;      // first prime in this thread's range
    uint192_t partial_square_sum; // sum of squares for this thread's primes only
    uint64_t  thread_prime_count;        // number of primes found by this thread
};

// next_gap_size = next modulus - start modulus
struct Checkpoint {
    uint64_t initial_prime;
    uint192_t initial_square_sum;   // value of S at beginning of thread sub interval
    uint64_t initial_prime_count;   // value of prime count at beginning of thread sub interval
    uint64_t thread_prime_count;   // number of primes in the thread's sub interval, used for feedback in estimating thread lengths
};

struct ThreadState {
    ThreadResult result;
    Checkpoint checkpoint;
    primesieve::iterator it;
    PrimeGapList gaps;

    explicit ThreadState(size_t initial_cap) {
        gaps.reserve(initial_cap);
    }
};

}   // namespace

#endif