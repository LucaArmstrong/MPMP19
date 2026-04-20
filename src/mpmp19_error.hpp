#ifndef MPMP19_ERROR_HPP
#define MPMP19_ERROR_HPP

#include <stdexcept>
#include <string>

namespace mpmp19 {

/// mpmp19 throws a mpmp19_error exception
/// if an error occurs e.g. a realloc fails
///
class mpmp19_error : public std::runtime_error {
    public:
        mpmp19_error(const std::string& msg) : std::runtime_error(msg) { }
};

} // namespace

#endif