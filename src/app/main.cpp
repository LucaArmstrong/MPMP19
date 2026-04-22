
#include <stdexcept>
#include <cstdio>
#include <cstdint>
#include <cerrno>
#include "mpmp19.hpp"

namespace {

constexpr uint64_t MILLION = 1000000ULL;
const char* PROGRESS_FILENAME = "progress.txt";
const char* TERM_FILENAME = "terms.txt";

}

int main(int argc, char** argv) {
    if (argc < 2) return EXIT_FAILURE;

    char *end;
    errno = 0;
    unsigned long long millions = strtoull(argv[1], &end, 10);

    if (errno != 0 || *end != '\0') return EXIT_FAILURE;
    if (millions > UINT64_MAX / MILLION) return EXIT_FAILURE;

    // optional parameter memory_mb
    if (argc >= 3) {
        errno = 0;
        uint32_t memory_mb = strtoul(argv[2], &end, 10);
        if (errno != 0 || *end != '\0' || memory_mb > UINT32_MAX) return EXIT_FAILURE;

        mpmp19::set_memory_mb(memory_mb);
    }

    // optional parameter num_threads
    if (argc >= 4) {
        errno = 0;
        uint32_t num_threads = strtoul(argv[3], &end, 10);
        if (errno != 0 || *end != '\0') return EXIT_FAILURE;

        mpmp19::set_num_threads(num_threads);
    }

    // run algorithm
    try {
        mpmp19::sequence(millions * MILLION, PROGRESS_FILENAME, TERM_FILENAME);
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return EXIT_FAILURE;
    }

    return 0;
}