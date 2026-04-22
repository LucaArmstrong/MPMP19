#ifndef TERM_BUFFER_HPP
#define TERM_BUFFER_HPP

#include <vector>
#include "BigInt.hpp"

namespace mpmp19 {

struct Term {
    uint64_t prime_count;
    uint128_t quotient;       // square_sum / prime_count
};

struct TermBuffer {
    std::vector<Term> data;
    
    int count() const;
    void add_term(uint64_t prime_count, uint128_t quotient);
    void reset();
    void sort();
    void output_terms(const char* filename) const;
};

}   // namespace

#endif