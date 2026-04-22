#ifndef BIG_INT_HPP
#define BIG_INT_HPP

#include <cstdint>
#include "Hints.hpp"

// Not supported on Windows MSVC compiler
#ifdef __SIZEOF_INT128__
  using uint128_t = __uint128_t;
#else
  #error "__uint128_t type not supported on this compiler!"
#endif

#ifndef UINT128_MAX
  #define UINT128_MAX ((uint128_t)-1)
#endif

namespace mpmp19 {

// store a 192-bit integer as a lower 128-bits and an upper 64-bits
// mathematically, uint192_t n = 2^128 * hi64 + lo128
struct uint192_t {
    uint64_t hi64;
    uint128_t lo128;
};

// returns the lower 64-bits of a 128-bit integer
inline uint64_t u128_lo(uint128_t n) {
    return (uint64_t)(n);
}

// returns the upper 64-bits of a 128-bit integer
inline uint64_t u128_hi(uint128_t n) {
    return (uint64_t)(n >> 64);
}

// takes a 192-bit integer a, and a 128-bit integer b as parameters
// returns the 192-bit integer a + b
inline uint192_t u192_add_u128(uint192_t a, uint128_t b) {
    uint192_t result;
    result.lo128 = a.lo128 + b;
    result.hi64 = a.hi64 + (result.lo128 < a.lo128);
    return result;
}

// takes a 192-bit integer n and a 64-bit integer d
// also takes pointers to a 128-bit integer q_out and a 64-bit integer r_out
// sets q_out to the quotient n / d and r_out to the remainder n % d
// this assumes that d is non-zero and the resulting quotient fits within a 128-bit unsigned integer
inline void u192_divmod_u64(uint192_t n, uint64_t d, uint128_t *q_out, uint64_t *r_out) {
    MPMP19_ASSERT(d != 0);
    MPMP19_ASSERT(n.hi64 < d);  // guarantees quotient fits in 128 bits
    
    // 2^128 = q1*d + r1
    uint128_t q1 = UINT128_MAX / d;
    uint64_t r1 = (UINT128_MAX % d + 1) % d;
    q1 += (r1 == 0);

    // n.low = q2*d + r2
    uint128_t q2 = n.lo128 / d;
    uint64_t r2 = n.lo128 % d;

    // initially n/d ~ q1*n.high + q2
    uint128_t q = (uint128_t)n.hi64 * q1 + q2;
    uint128_t r = (uint128_t)n.hi64 * (uint128_t)r1 +(uint128_t)r2;

    // leftover part
    *q_out = q + r / d;
    *r_out = r % d;
}

// takes two 192-bit integers a and b as parameters
// returns their 192-bit sum a + b
inline uint192_t u192_add_u192(uint192_t a, uint192_t b) {
    uint192_t result;
    result.lo128 = a.lo128 + b.lo128;
    result.hi64 = a.hi64 + b.hi64 + (result.lo128 < a.lo128);
    return result;
}

// returns the 192-bit representation of the integer 0
constexpr uint192_t u192_zero() {
    return {0, 0};
}

}   // namespace

#endif