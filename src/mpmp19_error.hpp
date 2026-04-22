#ifndef MPMP19_ERROR_HPP
#define MPMP19_ERROR_HPP

#include <stdexcept>
#include <string>

namespace mpmp19 {

// to be thrown on user based errors e.g. invalid inputs, files not opening
// not for implementation related issues
// these should be handled with assertions that are removed in the release build
class mpmp19_error : public std::runtime_error {
    public:
        mpmp19_error(const std::string& msg) : std::runtime_error(msg) { }
};

}

#endif