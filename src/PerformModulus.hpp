#ifndef PERFORM_MODULUS_HPP
#define PERFORM_MODULUS_HPP

#include "ThreadState.hpp"
#include "TermBuffer.hpp"
#include "PrimeGapIterator.hpp"

namespace mpmp19 {

// Every term n must be congruent to 1 or 5 mod 6.
// This is because (S(n) - n) = 5 mod 6
// meaning if n divides S(n) it must be coprime to 6.

// Some notation:
// n  : current divisor (prime count)
// Q  : quotient of S / n
// R  : remainder of S / n
// dS : increment in sum of squares
// g  : gap size (2 or 4 - to alternate n between 1 and 5 mod 6)
// t  : adjusted numerator for division step

// We maintain the invariant:
//   square_sum (S) = Q * n + R
// where:
//   Q = current quotient
//   R = current remainder (0 <= R < n)

// After adding dS to square_sum and increasing the divisor n by g,
// we compute:
//
//   t = R + dS − g*Q
//
// One can show that the value t will always be positive. Then:
//   Q <- Q + floor(t / n)
//   R <- t mod n
//
// This avoids recomputing the full division. Checking for a term becomes
// checking whether R = 0.

inline void perform_modulus_operations_thread(ThreadState& state, TermBuffer& results) {
    Checkpoint cp = state.checkpoint;
    PrimeGapList& gaps = state.gaps;
    PrimeGapIterator git = PrimeGapIterator(gaps);

    // extract initial checkpoint data
    uint64_t n = cp.initial_prime_count;
    uint64_t prime = cp.initial_prime;
    uint64_t primes_remaining = cp.thread_prime_count;
    uint192_t square_sum = cp.initial_square_sum;

    // we must align the modulus to be congruent to 1 or 5 mod 6
    // this allows us to step over two thirds of the values of n in the hot loop
    // using the 2-4 gap logic
    unsigned int nmod6 = n % 6;
    while (nmod6 != 1 && nmod6 != 5 && primes_remaining > 0) {
        prime += git.next_gap();
        uint128_t square = (uint128_t)prime * (uint128_t)prime;
        square_sum = u192_add_u128(square_sum, square);
        n++;
        nmod6 = (nmod6 + 1) % 6;
        primes_remaining--;
    }

    // Here we initialise Q and R using 192-bit division, and check for a term
    uint128_t Q;
    uint64_t R;
    u192_divmod_u64(square_sum, n, &Q, &R);

    if (R == 0)
        results.add_term(n, Q);

    unsigned int g = (nmod6 == 1) ? 4 : 2;

    // need i + (g - 1) < delta_list.length 
    // so that delta_S can be calculated for each iteration
    // otherwise, next 1 or 5 wouldn't be reached and it is safe to exit the loop
    while (primes_remaining >= g) {
        prime += git.next_gap();
        uint128_t delta_S = (uint128_t)prime * prime;
        prime += git.next_gap();
        delta_S += (uint128_t)prime * prime;
        
        if (g == 4) {
            prime += git.next_gap();
            delta_S += (uint128_t)prime * prime;
            prime += git.next_gap();
            delta_S += (uint128_t)prime * prime;
        }

        // obtain the value of t
        uint128_t gQ = (g == 2) ? (Q << 1) : (Q << 2);  // g*Q is either 2Q or 4Q
        uint128_t t = (uint128_t)R + delta_S - gQ;

        // update values involving g at the same time to reduce register spillage
        n += g;
        primes_remaining -= g;
        g = 6 - g;   // alternate g between 2 and 4

        // gcc compiler should see / and % and optimise to use __udivmodti4
        // which can be faster than computing quotient using __udivti3
        // and obtaining the remainder manually: t - q_inc * n
        uint128_t q_inc = t / n;
        R = t % n;
        Q += q_inc;

        if_unlikely(R == 0)
            results.add_term(n, Q);
    }
}

}   // namespace

#endif