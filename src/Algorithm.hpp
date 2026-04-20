#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include "Context.hpp"

namespace mpmp19 {

// Run the full sequence search — the main entry point
void run_sequence(Config& cfg, const char* progress_filename, const char* term_filename);

}

#endif