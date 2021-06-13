/*
 * This file is part of Dodecahedron
 *
 * Copyright (c) 2016, 2020 Alexander Kernozhitsky <sh200105@mail.ru>
 *
 * Dodecahedron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Dodecahedron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Dodecahedron.  If not, see <https://www.gnu.org/licenses/>.
 */

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
