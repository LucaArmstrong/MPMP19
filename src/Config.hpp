#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>

namespace mpmp19 {

struct Config {
    uint32_t memory_mb_;
    uint32_t num_threads_;
    uint64_t primes_per_interval_;
    uint64_t primes_per_thread_;
    uint64_t num_intervals_;

    Config(uint64_t N, uint32_t memory_mb, uint32_t num_threads);
};

}

#endif