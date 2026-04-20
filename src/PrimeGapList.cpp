#include "PrimeGapList.hpp"

#include <memory>
#include "mpmp19_error.hpp"
#include "Hints.hpp"

namespace mpmp19 {

PrimeGapList::~PrimeGapList() {
    std::free(data);
}

void PrimeGapList::reset() {
    length = 0;
}

void PrimeGapList::reserve(size_t cap_) {
    if (cap_ <= cap) return;

    uint8_t* data_ = static_cast<uint8_t*>(std::realloc(data, cap_ * sizeof(uint8_t)));
    
    if (data_ == nullptr)
        throw mpmp19_error("PrimeGapList realloc failed: out of memory");
    
    data = data_;
    cap = cap_;
}

// Ensure room for one more full primesieve batch (MAX_PRIMESIEVE_PRIMES elements).
// Call once before writing a batch directly to list->data + list->length.
// After writing, manually increment list->length by the number of elements written.
void PrimeGapList::reserve_full_batch() {
    if_likely(this->length + MAX_PRIMES_PER_BATCH * 3 <= cap) return;
    reserve(cap + MAX_PRIMES_PER_BATCH * 3);
}

void PrimeGapList::push(uint16_t gap) {
    if_likely(gap < LARGE_GAP_THRESHOLD) {
        MPMP19_ASSERT(length < cap);
        data[length++] = (uint8_t)(gap);
    } else {
        MPMP19_ASSERT(length + 2 < cap);
        data[length++] = LARGE_GAP_FLAG;
        data[length++] = (uint8_t)(gap >> 8);
        data[length++] = (uint8_t)(gap);
    }
}

}   // namespace