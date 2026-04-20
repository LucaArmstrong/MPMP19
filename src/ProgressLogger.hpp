#ifndef PROGRESS_LOGGER_HPP
#define PROGRESS_LOGGER_HPP

#include <cstdint>
#include "Config.hpp"

namespace mpmp19 {

void initialise_output_file(const char* filename);
void log_start_config(const Config& cfg, const char* filename);
void log_progress(uint64_t& billion_count, uint64_t prime_count, double start_time, double& last_time, const char* filename);
void log_final_stats(uint64_t prime_count, uint32_t term_count, double total_time, double sequence_time, double mod_time, const char* filename);
double now_seconds();

}

#endif