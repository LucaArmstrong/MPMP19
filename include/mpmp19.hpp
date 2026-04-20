
#ifndef MPMP19_HPP
#define MPMP19_HPP

#include <cstdint>

namespace mpmp19 {

void set_memory_mb(uint32_t mb);
void set_num_threads(uint32_t n);

void sequence(uint64_t N, const char* progress_filename, const char* term_filename);

}

#endif
