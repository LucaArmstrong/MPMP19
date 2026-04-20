#include "Config.hpp"

#include "mpmp19_error.hpp"
#include "BigInt.hpp"

namespace mpmp19 {

// the maximum limit is derived from the delta_S variable in perform_modulus_operations_thread()
// in src/Algorithm.cpp. Namely, the value of delta_S must stay below 2^128. If n denotes the largest 
// prime count that the program can have and p(k) denotes the, kth prime, then we obtain an upper bound
//
//           delta_S <= p(n-3)^2 + p(n-2)^2 + p(n-1)^2 + p(n-0)^2 < 4 * p(n)^2 < 2^128
//
//                            p(n) < 2^63
//                            n < PrimePi(2^63) = 216289611853439384
//                            n <= 216289611853439383
constexpr uint64_t MAX_NUMBER = 216289611853439383ULL;

Config::Config(uint64_t N, uint32_t memory_mb_, uint32_t num_threads_)
    : memory_mb(memory_mb_)
    , num_threads(num_threads_)
{
    const uint64_t bytes = (uint64_t)memory_mb * (1ULL << 20);
    primes_per_thread_interval = bytes / num_threads;
    primes_per_interval = primes_per_thread_interval * num_threads;

    if (primes_per_thread_interval == 0)
        throw mpmp19_error("Interval memory is too low for the prime storage!");
    
    // Compute number of intervals needed to cover N primes.
    // Uses ceiling division: ceil(N / primes_per_interval) = (N-1)/k + 1
    // The N=0 edge case is handled explicitly to avoid underflow on (N-1).
    num_intervals = (N == 0) ? 0 : (N - 1) / primes_per_interval + 1;

    const uint128_t limit = (uint128_t)num_intervals * primes_per_interval;
    if (limit > (uint128_t)MAX_NUMBER)
        throw mpmp19_error("Limit exceeds maximum allowed value!");
}

}   // namespace