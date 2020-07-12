#ifndef SOF_UTIL_COMPILER_INCLUDED
#define SOF_UTIL_COMPILER_INCLUDED

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif  // SOF_UTIL_COMPILER_INCLUDED
