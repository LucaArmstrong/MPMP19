#ifndef PRIME_GAP_ITERATOR_HPP
#define PRIME_GAP_ITERATOR_HPP

#include <cstdint>
#include "PrimeGapList.hpp"
#include "Hints.hpp"

namespace mpmp19 {

struct PrimeGapIterator {
    const uint8_t* data_;   // we only ever read from this array
    const size_t size_;     // number of bytes used to store gaps
    size_t i_ = 0;          // byte position in the array

    explicit PrimeGapIterator(const PrimeGapList& gap_list) 
        : data_(gap_list.data_)
        , size_(gap_list.size_) {}

    inline uint16_t next_gap() {
        MPMP19_ASSERT(i_ < size_);
        uint16_t gap = data_[i_++];
        if_unlikely(gap == PrimeGapList::LARGE_GAP_FLAG) {
            MPMP19_ASSERT(i_ + 1 < size_);
            gap = ((uint16_t)data_[i_] << 8) | data_[i_ + 1];
            i_ += 2;
        }
        return gap;
    }
};

}   // namespace

#endif