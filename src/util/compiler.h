#ifndef COMPILER_H_INCLUDED
#define COMPILER_H_INCLUDED

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif  // COMPILER_H_INCLUDED
