#include "TermBuffer.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>
#include <cinttypes>

#include "FileHandle.hpp"
#include "BigInt.hpp"

namespace mpmp19 {

void TermBuffer::add_term(uint64_t prime_count, uint128_t quotient) {
    // omp critical is generally bad for performance but is fine here due to there being so few terms
    // As of 11/04/2026 only 16 terms in the OEIS sequence A111441 are known
    #pragma omp critical
    {
        Term new_term = {prime_count, quotient};
        data.push_back(new_term);
    }
}

void TermBuffer::reset() {
    data.clear();
}

uint32_t TermBuffer::count() const {
    return (uint32_t)data.size();
}

void TermBuffer::sort() {
    if (count() == 0) return;
    
    std::sort(data.begin(), data.end(), [](const Term& a, const Term& b) {
        return a.prime_count < b.prime_count;
    });
}

void TermBuffer::output_terms(const char* filename) const {
    if (count() == 0) return;    // return early if no new terms were found

    FileHandle file(filename, "a");

    // uint128_t has no printf format specifier, so we print as two 64-bit halves.
    // To reconstruct: quotient = (upper << 64) | lower
    for (const Term& term : data) {
        fprintf(file.ptr, "Prime count:\t%" PRIu64 "\n", term.prime_count);
        fprintf(file.ptr, "Quotient upper:\t%" PRIu64 "\n", u128_hi(term.quotient));
        fprintf(file.ptr, "Quotient lower:\t%" PRIu64 "\n\n", u128_lo(term.quotient));
    }
}

}   // namespace