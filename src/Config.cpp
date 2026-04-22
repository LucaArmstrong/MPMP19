#include "Config.hpp"

#include "mpmp19_error.hpp"
#include "BigInt.hpp"

namespace mpmp19 {

// MAX_NUMBER is derived from the quantity t in PerformModulus.hpp
// t = R + dS - gQ can be bound above by 4*p(n)^2
// As t must fit within 128 bits, we can say that 4*p(n)^2 < 2^128
// or equivalently, p(n) < 2^63 => n <= PrimePi(2^63) ~ 2.16*10^17
// Reference for value: https://oeis.org/A007053
constexpr uint64_t MAX_NUMBER = 216289611853439384ULL;

Config::Config(uint64_t N, uint32_t memory_mb_, uint32_t num_threads_)
    : memory_mb(memory_mb_)
    , num_threads(num_threads_)
{
    const uint64_t bytes = (uint64_t)memory_mb * (1ULL << 20);
    primes_per_thread = bytes / num_threads;
    primes_per_interval = primes_per_thread * num_threads;

    if (primes_per_thread == 0)
        throw mpmp19_error("Interval memory is too low for the prime storage!");
    
    // Compute ceil(N / primes_per_interval)
    // ~ the number of intervals needed to cover N primes
    num_intervals = (N == 0) ? 0 : (N - 1) / primes_per_interval + 1;

    const uint128_t limit = (uint128_t)num_intervals * primes_per_interval;
    if (limit > (uint128_t)MAX_NUMBER)
        throw mpmp19_error("Limit exceeds maximum allowed value!");
}

}   // namespace