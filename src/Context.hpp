#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <vector>
#include "Config.hpp"
#include "ThreadState.hpp"
#include "BigInt.hpp"

namespace mpmp19 {

struct Context {
    Config cfg;
    uint192_t square_sum = u192_zero();
    uint64_t prime_count = 0;
    uint32_t term_count = 0;
    std::vector<ThreadState> thread_states;

    Context(Config cfg_) : cfg(cfg_)
    {
        thread_states.reserve(cfg.num_threads);
        const size_t initial_cap = cfg.primes_per_thread_interval;

        for (size_t i = 0; i < cfg.num_threads; i++)
            thread_states.emplace_back(initial_cap);
    }

    // Non-copyable — owns thread state
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
};

} // namespace mpmp19

#endif