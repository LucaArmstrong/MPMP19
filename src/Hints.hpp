#ifndef HINTS_HPP
#define HINTS_HPP

#ifndef __has_cpp_attribute
  #define __has_cpp_attribute(x) 0
#endif

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#if __cplusplus >= 202002L && __has_cpp_attribute(likely)
  #define if_likely(x) if (x) [[likely]]
#elif __has_builtin(__builtin_expect) || defined(__GNUC__)
  #define if_likely(x) if (__builtin_expect(!!(x), 1))
#else
  #define if_likely(x) if (x)
#endif

#if __cplusplus >= 202002L && __has_cpp_attribute(unlikely)
  #define if_unlikely(x) if (x) [[unlikely]]
#elif __has_builtin(__builtin_expect) || defined(__GNUC__)
  #define if_unlikely(x) if (__builtin_expect(!!(x), 0))
#else
  #define if_unlikely(x) if (x)
#endif

// MSVC, GNU, Clang and Intel compilers support the restrict keyword in C++
// For other compilers we just ignore RESTRICT
#ifdef _MSC_VER
  #define RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
  #define RESTRICT __restrict__
#else
  #define RESTRICT
#endif

// When debug mode is enabled, assertions are added to things like bound checks and validation
#ifndef NDEBUG
  #include <cassert>
  #define MPMP19_ASSERT(x) assert(x)
#else
  #define MPMP19_ASSERT(x) ((void)0)
#endif

#endif