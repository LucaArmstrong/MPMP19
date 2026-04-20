#ifndef HINTS_HPP
#define HINTS_HPP

#if __cplusplus >= 202002L && \
    __has_cpp_attribute(likely)
  #define if_likely(x) if (x) [[likely]]
#elif defined(__GNUC__) || \
      __has_builtin(__builtin_expect)
  #define if_likely(x) if (__builtin_expect(!!(x), 1))
#else
  #define if_likely(x) if (x)
#endif

#if __cplusplus >= 202002L && \
    __has_cpp_attribute(unlikely)
  #define if_unlikely(x) if (x) [[unlikely]]
#elif defined(__GNUC__) || \
      __has_builtin(__builtin_expect)
  #define if_unlikely(x) if (__builtin_expect(!!(x), 0))
#else
  #define if_unlikely(x) if (x)
#endif

#ifdef _MSC_VER
  #define RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
  #define RESTRICT __restrict__
#else
  #define RESTRICT
#endif

#ifndef NDEBUG
  #include <cassert>
  #define MPMP19_ASSERT(x) assert(x)
#else
  #define MPMP19_ASSERT(x) ((void)0)
#endif

#endif