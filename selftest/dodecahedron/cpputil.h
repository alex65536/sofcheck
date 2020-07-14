/*
    cpputil.h is created to call standard C++ methods.
    IT'S RECOMMENDED TO USE METHODS FROM HERE AND NOT TO USE STANDARD C++ HEADERS !!!
*/
#ifndef CPPUTIL_H_INCLUDED
#define CPPUTIL_H_INCLUDED

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

inline int popcount(int64_t);
#include "bitutil.h"

char* int_to_str(int64_t, char*);

void out_str(const char*);
void out_char(char);
void out_int(int64_t);
void out_fmt(const char*, ...);
void out_vfmt(const char*, va_list);

void in_line(char*, int max_len = 256);
void out_line(const char*);
void out_line_fmt(const char*, ...);

void randomize64();
int64_t random64();
int64_t random64(int64_t); // [0 .. r-1]
int64_t random64(int64_t, int64_t); // [l .. r]

#ifdef DEBUG
    void fatal_error(const char*);
#else
    #define fatal_error(s)
#endif

#endif // CPPUTIL_H_INCLUDED
