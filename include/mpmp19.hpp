
#ifndef MPMP19_HPP
#define MPMP19_HPP

#include <cstdint>

namespace mpmp19 {

// determines the amount of memory in megabytes allocated to storing prime gaps
// in reality, the program may use 1-2% more than this amount
// default value is 1000 MB = 1 GB
void set_memory_mb(uint32_t mb);

// by default the number of threads is set to omp_get_max_threads()
void set_num_threads(uint32_t n);

// run the algorithm to find terms in the sequence A111441 up to the value N
// outputs progress to progress_filename and stdout
// any terms found are stored in term_filename
void sequence(uint64_t N, const char* progress_filename, const char* term_filename);

}

#endif
