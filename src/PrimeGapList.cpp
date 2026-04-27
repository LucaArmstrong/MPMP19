#include "PrimeGapList.hpp"

#include <memory>
#include "mpmp19_error.hpp"
#include "Hints.hpp"

namespace mpmp19 {

PrimeGapList::~PrimeGapList() {
    std::free(data_);
}

void PrimeGapList::reset() {
    size_ = 0;
}

void PrimeGapList::reserve(size_t cap) {
    if (cap <= cap_) return;

    uint8_t* data = static_cast<uint8_t*>(std::realloc(data_, cap));
    
    if (data == nullptr)
        throw mpmp19_error("PrimeGapList: memory allocation failed");
    
    data_ = data;
    cap_ = cap;
}

// Ensure room for one more full primesieve batch (MAX_PRIMESIEVE_PRIMES elements).
// Call once before writing a batch directly to list.data + list.size.
void PrimeGapList::reserve_full_batch() {
    if_likely(size_ + MAX_PRIMES_PER_BATCH * 3 <= cap_) return;
    reserve(cap_ + cap_ / 100 + MAX_PRIMES_PER_BATCH * 3);
}

void PrimeGapList::push(uint16_t gap) {
    if_likely(gap < LARGE_GAP_THRESHOLD) {
        MPMP19_ASSERT(size_ < cap_);
        data_[size_++] = (uint8_t)(gap);
    } else {
        MPMP19_ASSERT(size_ + 2 < cap_);
        data_[size_++] = LARGE_GAP_FLAG;
        data_[size_++] = (uint8_t)(gap >> 8);
        data_[size_++] = (uint8_t)(gap);
    }
}

}   // namespace