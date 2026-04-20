#ifndef PERFORM_MODULUS_HPP
#define PERFORM_MODULUS_HPP

#include "ThreadState.hpp"
#include "TermBuffer.hpp"
#include "PrimeGapIterator.hpp"
#include "Hints.hpp"

namespace mpmp19 {

/*inline constexpr uint64_t THRESHOLD = 1ULL << 35;

struct reciprocal_data {
    // g = 2
    uint64_t gu1_doubled_2;
    uint64_t difference_2;      // g*u1^2 - g^2 * u1^3 / 2^64

    // g = 4
    uint64_t gu1_doubled_4;
    uint64_t difference_4;

    inline void set_data(uint64_t u1) {
        uint64_t gu1_2 = u1 << 1;
        uint64_t gu1_4 = u1 << 2;

        gu1_doubled_2 = gu1_2 << 1;
        gu1_doubled_4 = gu1_4 << 1;

        uint64_t part3_2 = ((uint128_t)gu1_2 * gu1_2 * u1) >> 64;
        uint64_t part3_4 = ((uint128_t)gu1_4 * gu1_4 * u1) >> 64;
        
        difference_2 = gu1_2 * u1 - part3_2;
        difference_4 = gu1_4 * u1 - part3_4;
    }
};

inline void advance_reciprocal_fast(uint128_t& u, uint64_t u2, uint64_t new_m, uint64_t g, reciprocal_data& data) {
    if (g == 2) {
        u -= data.difference_2;
        u -= ((uint128_t)data.gu1_doubled_2 * u2) >> 64;
    } else {
        u -= data.difference_4;
        u -= ((uint128_t)data.gu1_doubled_4 * u2) >> 64;
    }

    // now correct u
    // should have 2^128 / m = u
    uint128_t rem = -(u * new_m);
    
    if (rem > UINT64_MAX) {
        do {
            rem += new_m;
            u--;
        } while (rem > UINT64_MAX);
    } else if (rem >= new_m) {
        do {
            rem -= new_m;
            u++;
        } while (rem >= new_m);
    }
}

inline void perform_modulus_operations_fast(ThreadState& state, TermBuffer& results) {
    Checkpoint cp = state.checkpoint;
    PrimeGapList& gaps = state.gaps;
    PrimeGapIterator git = PrimeGapIterator(gaps);

    // extract checkpoint data
    uint64_t m = cp.initial_prime_count;
    uint64_t prime = cp.initial_prime;
    uint64_t primes_remaining = cp.thread_prime_count;
    uint192_t square_sum = cp.initial_square_sum;

    // we must align the modulus to be congruent to 1 or 5 mod 6
    // this allows us to step over 2/3's of the values of m in the hot loop
    // using the 2-4 gap logic
    uint32_t mod6 = m % 6;
    while (mod6 != 1 && mod6 != 5 && primes_remaining > 0) {
        prime += git.next_gap();
        uint128_t square = (uint128_t)prime * (uint128_t)prime;
        square_sum = u192_add_u128(square_sum, square);
        m++;
        mod6 = (mod6 + 1) % 6;
        primes_remaining--;
    }

    // note that primes_remaining could be zero here, but m could still equal 1 or 5
    // mod 6 so we must still check for a term here.
    // Q and R denote the quotient and remainder of square_sum divided by m respectively
    uint128_t Q;
    uint64_t R1;
    u192_divmod_u64(square_sum, m, &Q, &R1);

    if (R1 == 0)
        results.add_term(m, Q);
    
    int64_t R = R1;

    // now if no more primes left after the alignment we can safely
    // return from the function - note that this would never happen in reality
    // as primesieve will always give us at least 6 primes.
    // But it isn't impossible.
    if (primes_remaining == 0) return;

    uint32_t g = (mod6 == 1) ? 4 : 2;   // g will only ever alternate between 2 and 4

    uint128_t u;
    uint64_t v;
    u192_divmod_u64({1, 0}, m, &u, &v);

    uint64_t u1 = u >> 64;
    uint64_t u2 = (uint64_t)u;

    reciprocal_data data;
    data.set_data(u1);

    // need i + (g - 1) < delta_list.length 
    // so that delta_S can be calculated for each iteration
    // otherwise, next 1 or 5 wouldn't be reached and it is safe to exit the loop
    while (primes_remaining >= g) {
        // recalculate the sum of squares of the primes
        prime += git.next_gap();
        uint128_t delta_S = (uint128_t)prime * (uint128_t)prime;
        prime += git.next_gap();
        delta_S += (uint128_t)prime * (uint128_t)prime;
        
        uint128_t n;

        if (g == 4) {
            prime += git.next_gap();
            delta_S += (uint128_t)prime * (uint128_t)prime;
            prime += git.next_gap();
            delta_S += (uint128_t)prime * (uint128_t)prime;

            // using recurrence to update the quotient and remainder values 
            // Q and R
            uint128_t gQ = Q << 2;  // g*Q is either 2Q or 4Q
            n = (uint128_t)R + delta_S - gQ;

            m += 4;
            primes_remaining -= 4;
            g = 2;

            // ADVANCE RECIPROCAL
            u -= data.difference_4;
            u -= ((uint128_t)data.gu1_doubled_4 * u2) >> 64;
        } else {
            // using recurrence to update the quotient and remainder values 
            // Q and R
            uint128_t gQ = Q << 1;  // g*Q is either 2Q or 4Q
            n = (uint128_t)R + delta_S - gQ;

            m += 2;
            primes_remaining -= 2;
            g = 4;

            // ADVANCE RECIPROCAL
            u -= data.difference_2;
            u -= ((uint128_t)data.gu1_doubled_2 * u2) >> 64;
        }

        // correct u
        uint128_t rem = -(u * m);
        
        if (rem > UINT64_MAX) {
            do {
                rem += m;
                u--;
            } while (rem > UINT64_MAX);
        } else if (rem >= m) {
            do {
                rem -= m;
                u++;
            } while (rem >= m);
        }
        
        // n / m ~ n * u / 2^128
        uint64_t n1 = n >> 64;
        uint64_t n2 = (uint64_t)n;

        uint64_t u1_new = u >> 64;
        u2 = (uint64_t)u;

        if_unlikely(u1_new != u1) {
            data.set_data(u1_new);
            u1 = u1_new;
        }

        // calculate floor(n * u / 2^128) = a
        uint128_t a = (uint128_t)n1 * u2 + (uint128_t)n2 * u1;
        a >>= 64;
        a += (uint128_t)n1 * u1;

        R = n - a * m;

        if (R >= (int64_t)m) {
            R -= m;
            a++;
        }

        Q += a;

        if_unlikely(R == 0)
            results.add_term(m, Q);
    }
}*/

inline void perform_modulus_operations_thread(ThreadState& state, TermBuffer& results) {
    Checkpoint cp = state.checkpoint;
    PrimeGapList& gaps = state.gaps;
    PrimeGapIterator git = PrimeGapIterator(gaps);

    // extract checkpoint data
    uint64_t m = cp.initial_prime_count;
    uint64_t prime = cp.initial_prime;
    uint64_t primes_remaining = cp.thread_prime_count;
    uint192_t square_sum = cp.initial_square_sum;

    // we must align the modulus to be congruent to 1 or 5 mod 6
    // this allows us to step over 2/3's of the values of m in the hot loop
    // using the 2-4 gap logic
    uint32_t mod6 = m % 6;
    while (mod6 != 1 && mod6 != 5 && primes_remaining > 0) {
        prime += git.next_gap();
        uint128_t square = (uint128_t)prime * (uint128_t)prime;
        square_sum = u192_add_u128(square_sum, square);
        m++;
        mod6 = (mod6 + 1) % 6;
        primes_remaining--;
    }

    // note that primes_remaining could be zero here, but m could still equal 1 or 5
    // mod 6 so we must still check for a term here.
    // Q and R denote the quotient and remainder of square_sum divided by m respectively
    uint128_t Q;
    uint64_t R;
    u192_divmod_u64(square_sum, m, &Q, &R);

    if (R == 0)
        results.add_term(m, Q);

    // now if no more primes left after the alignment we can safely
    // return from the function - note that this would never happen in reality
    // as primesieve will always give us at least 6 primes.
    // But it isn't impossible.
    if (primes_remaining == 0) return;

    uint32_t g = (mod6 == 1) ? 4 : 2;   // g will only ever alternate between 2 and 4

    // need i + (g - 1) < delta_list.length 
    // so that delta_S can be calculated for each iteration
    // otherwise, next 1 or 5 wouldn't be reached and it is safe to exit the loop
    while (primes_remaining >= g) {
        // recalculate the sum of squares of the primes
        prime += git.next_gap();
        uint128_t delta_S = (uint128_t)prime * (uint128_t)prime;
        prime += git.next_gap();
        delta_S += (uint128_t)prime * (uint128_t)prime;
        
        if (g == 4) {
            prime += git.next_gap();
            delta_S += (uint128_t)prime * (uint128_t)prime;
            prime += git.next_gap();
            delta_S += (uint128_t)prime * (uint128_t)prime;
        }

        // using recurrence to update the quotient and remainder values 
        // Q and R
        uint128_t gQ = (g == 2) ? (Q << 1) : (Q << 2);  // g*Q is either 2Q or 4Q
        uint128_t n = (uint128_t)R + delta_S - gQ;

        // update modulus and do all g operations to reduce register spillage
        m += g;
        primes_remaining -= g;
        g = 6 - g;   // alternate g between 2 and 4
        
        // compiler should see this and optimise to use __udivmodti4
        // which is (slightly) faster than obtaining the remainder manually
        // i.e. n - n_quotient * m, in which case the compiler would choose
        // __udivti3 instead
        uint128_t n_quotient = n / m;
        R = n % m;
        Q += n_quotient;

        if_unlikely(R == 0)
            results.add_term(m, Q);
    }
}

}   // namespace

#endif