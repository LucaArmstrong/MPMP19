#ifndef PROCESS_BATCH_HPP
#define PROCESS_BATCH_HPP

#include <cstdint>
#include <primesieve/iterator.hpp>

#include "BigInt.hpp"
#include "PrimeGapList.hpp"
#include "Hints.hpp"

namespace mpmp19 {

inline uint64_t process_full_batch(const primesieve::iterator& it, PrimeGapList& gaps, uint192_t& square_sum, uint64_t& prev_prime) {
    gaps.reserve_full_batch();

    // cache values in registers to avoid memory calls
    uint192_t batch_sum = u192_zero();
    uint64_t p_prev = prev_prime;

    const size_t count = it.size_;
    const uint64_t* RESTRICT primes = it.primes_;
    uint8_t* RESTRICT data = gaps.data;
    size_t length = gaps.length;

    size_t i = 0;
    while (i + 4 <= count) {
        uint64_t p0 = primes[i+0];
        uint64_t p1 = primes[i+1];
        uint64_t p2 = primes[i+2];
        uint64_t p3 = primes[i+3];
        i += 4;

        uint128_t delta_S = (uint128_t)p0 * p0
                          + (uint128_t)p1 * p1
                          + (uint128_t)p2 * p2
                          + (uint128_t)p3 * p3;

        batch_sum = u192_add_u128(batch_sum, delta_S);

        uint16_t g0 = (uint16_t)(p0 - p_prev);
        uint16_t g1 = (uint16_t)(p1 - p0);
        uint16_t g2 = (uint16_t)(p2 - p1);
        uint16_t g3 = (uint16_t)(p3 - p2);
        p_prev = p3;
    
        // do what is most likely first (very fast)
        data[length+0] = (uint8_t)g0;
        data[length+1] = (uint8_t)g1;
        data[length+2] = (uint8_t)g2;
        data[length+3] = (uint8_t)g3;

        // compound condition is still extremely likely
        // faster to evaluate that 4 separate conditions and branches
        if_likely(((g0 | g1 | g2 | g3) & PrimeGapList::LARGE_GAP_MASK) == 0) {
            length += 4;
        } else {
            // do manual process like before
            if (g0 < PrimeGapList::LARGE_GAP_THRESHOLD) {data[length++] = (uint8_t)g0;} 
            else {data[length++] = PrimeGapList::LARGE_GAP_FLAG; data[length++] = (uint8_t)(g0 >> 8); data[length++] = (uint8_t)g0;}

            if (g1 < PrimeGapList::LARGE_GAP_THRESHOLD) {data[length++] = (uint8_t)g1;} 
            else {data[length++] = PrimeGapList::LARGE_GAP_FLAG; data[length++] = (uint8_t)(g1 >> 8); data[length++] = (uint8_t)g1;}

            if (g2 < PrimeGapList::LARGE_GAP_THRESHOLD) {data[length++] = (uint8_t)g2;} 
            else {data[length++] = PrimeGapList::LARGE_GAP_FLAG; data[length++] = (uint8_t)(g2 >> 8); data[length++] = (uint8_t)g2;}

            if (g3 < PrimeGapList::LARGE_GAP_THRESHOLD) {data[length++] = (uint8_t)g3;} 
            else {data[length++] = PrimeGapList::LARGE_GAP_FLAG; data[length++] = (uint8_t)(g3 >> 8); data[length++] = (uint8_t)g3;}
        }
    }

    gaps.length = length;

    for (; i < count; i++) {
        uint64_t p = primes[i];
        uint128_t square = (uint128_t)p * (uint128_t)p;
        batch_sum = u192_add_u128(batch_sum, square);
        uint16_t gap = (uint16_t)(p - p_prev);

        gaps.push(gap);

        p_prev = p; 
    }

    prev_prime = p_prev;
    square_sum = u192_add_u192(square_sum, batch_sum);
    return it.size_;
}

inline uint64_t process_partial_batch(const primesieve::iterator& it, PrimeGapList& gaps, uint192_t& square_sum, uint64_t& prev_prime, uint64_t end_number) {
    gaps.reserve_full_batch();

    // cache values in registers to avoid memory calls
    uint192_t batch_sum = u192_zero();
    uint64_t p_prev = prev_prime;
    uint64_t batch_prime_count = 0;
    
    // partial batch — only copy primes up to end_number
    for (size_t i = 0; i < it.size_ && it.primes_[i] <= end_number; i++) {
        uint64_t p = it.primes_[i];
        uint128_t square = (uint128_t)p * (uint128_t)p;
        batch_sum = u192_add_u128(batch_sum, square);
        uint16_t gap = (uint16_t)(p - p_prev);

        gaps.push(gap);

        p_prev = p;
        batch_prime_count++;
    }

    prev_prime = p_prev;
    square_sum = u192_add_u192(square_sum, batch_sum);
    return batch_prime_count;
}

}   // namespace

#endif