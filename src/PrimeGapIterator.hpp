#ifndef PRIME_GAP_ITERATOR_HPP
#define PRIME_GAP_ITERATOR_HPP

#include <cstdint>
#include "PrimeGapList.hpp"
#include "mpmp19_error.hpp"
#include "Hints.hpp"

namespace mpmp19 {

struct PrimeGapIterator {
    const uint8_t* data;   // const - we only read
    size_t i = 0;     // byte position
    size_t length;

    PrimeGapIterator(const PrimeGapList& gap_list) {
        data = gap_list.data;
        length = gap_list.length;
    }

    inline uint16_t next_gap() {
        MPMP19_ASSERT(i < length);
        uint16_t gap = data[i++];
        if_unlikely(gap == PrimeGapList::LARGE_GAP_FLAG) {
            MPMP19_ASSERT(i + 1 < length);
            gap = ((uint16_t)data[i] << 8) | data[i + 1];
            i += 2;
        }
        return gap;
    }
};

}   // namespace

#endif