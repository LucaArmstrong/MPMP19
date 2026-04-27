#include "ProgressLogger.hpp"

#include <chrono>
#include <string>
#include <cinttypes>
#include <cstdint>
#include <cstdio>

#include "FileHandle.hpp"
#include "Config.hpp"
#include "Hints.hpp"

namespace {

// BILLION is used for tracking the progress of the program
// i.e. every 10^9 primes the elapsed time and time since the last billion is outputted
constexpr uint64_t BILLION = 1000000000ULL;

void report_start(FILE* out, const mpmp19::Config& cfg) {
    fprintf(out, "Number of threads = %" PRIu32 "\n", cfg.num_threads_);
    fprintf(out, "Interval memory = %" PRIu64 " MB\n", (uint64_t)cfg.memory_mb_);
    fprintf(out, "Searching the first %" PRIu64 " intervals\n", cfg.num_intervals_);
    fprintf(out, "Target: %" PRIu64 " primes\n\n", cfg.num_intervals_ * cfg.primes_per_interval_);
}

void report_progress(FILE* out, uint64_t billion_count, double elapsed, double interval) {
    fprintf(out, "At %2" PRIu64 " billion - elapsed time: %7.3fs  interval: %6.3fs\n", billion_count, elapsed, interval);
}

void report_end(FILE* out, uint64_t prime_count, uint32_t term_count, double total_time, double sequence_time, double mod_time) {
    // compute percentages
    const double sequence_pct = 100.0 * sequence_time / total_time;
    const double mod_pct = 100.0 * mod_time / total_time;

    fprintf(out, "\nDONE\n");
    fprintf(out, "%-28s %" PRIu64 "\n", "Primes searched:", prime_count);
    fprintf(out, "%-28s %" PRIu32 "\n\n", "Terms found:", term_count);
    
    fprintf(out, "%-28s %.3fs\n", "Total computation time:", total_time);
    fprintf(out, "%-28s %.3fs (%.1f%%)\n", "Sequencing time:", sequence_time, sequence_pct);
    fprintf(out, "%-28s %.3fs (%.1f%%)\n", "Modulus check time:", mod_time, mod_pct);
}

}   // namespace

namespace mpmp19 {

// opening a file in write mode either creates a new file with that name, or clears and existing one
void initialise_output_file(const char* filename) {
    FileHandle file(filename, "w");
}

void log_start_config(const Config& cfg, const char* filename) {
    FileHandle file(filename, "a");
    report_start(file.ptr, cfg);
    report_start(stdout, cfg);
}

void log_progress(uint64_t& billion_count, uint64_t prime_count, double start_time, double& last_time, const char* filename) {
    if (prime_count / BILLION <= billion_count) return;
    billion_count = prime_count / BILLION;

    double current_time = now_seconds();
    double elapsed = current_time - start_time;
    double interval = current_time - last_time;
    last_time = current_time;

    FileHandle file(filename, "a");
    report_progress(file.ptr, billion_count, elapsed, interval);
    report_progress(stdout, billion_count, elapsed, interval);
}

void log_final_stats(uint64_t prime_count, uint32_t term_count, double total_time, double sequence_time, double mod_time, const char* filename) {
    FileHandle file(filename, "a");
    report_end(file.ptr, prime_count, term_count, total_time, sequence_time, mod_time);
    report_end(stdout, prime_count, term_count, total_time, sequence_time, mod_time);
}

double now_seconds() {
    using clock = std::chrono::steady_clock;
    auto now = clock::now().time_since_epoch();
    return std::chrono::duration<double>(now).count();
}

}   // namespace