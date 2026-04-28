#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <vector>
#include "Config.hpp"
#include "ThreadState.hpp"
#include "BigInt.hpp"

namespace mpmp19 {

struct Context {
    Config cfg_;
    uint192_t square_sum_ = u192_zero();
    uint64_t prime_count_ = 0;
    uint32_t term_count_ = 0;
    std::vector<ThreadState> thread_states_;

    explicit Context(Config cfg) : cfg_(std::move(cfg))
    {
        const size_t initial_cap = cfg_.primes_per_thread_;

        thread_states_.reserve(cfg_.num_threads_);
        for (size_t i = 0; i < cfg_.num_threads_; i++)
            thread_states_.emplace_back(initial_cap);
    }
};

}   // namespace

#endif