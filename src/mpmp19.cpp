#include "mpmp19.hpp"

#include <algorithm>
#include <omp.h>

#include "Algorithm.hpp"
#include "ThreadState.hpp"
#include "Context.hpp"
#include "Config.hpp"

namespace {

uint32_t memory_mb = 1000;   // 1 GB is used by default
uint32_t num_threads = 0;    // here 0 means use maximum number of threads available

}

namespace mpmp19 {

void set_memory_mb(uint32_t mb) {
    if (mb > 0)
        memory_mb = mb;
}

void set_num_threads(uint32_t n) {
    num_threads = n;
}

void sequence(uint64_t N, const char* progress_filename, const char* term_filename) {
    uint32_t max_threads = omp_get_max_threads();
    uint32_t threads = (num_threads == 0) ? max_threads : std::min(max_threads, num_threads);

    Config cfg(N, memory_mb, threads);
    run_sequence(cfg, progress_filename, term_filename);
}

}   // namespace