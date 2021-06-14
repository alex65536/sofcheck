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
#ifndef BITUTIL_H_INCLUDED
#define BITUTIL_H_INCLUDED

#ifdef __GNUG__
    inline int popcount(int64_t b) { return __builtin_popcountll(b); }
    #define HAS_POPCOUNT
#endif // __GNUG__
#if defined(_WIN64) && defined(_MSC_VER) && _MSC_VER >= 1400
    #include <intrin.h>
    inline int popcount(int64_t b) { return __popcnt64(b); }
    #define HAS_POPCOUNT
#endif // defined(_WIN64) && defined(_MSC_VER) && _MSC_VER >= 1400
#ifndef HAS_POPCOUNT
    #include <limits>
    #include <bitset>
    inline int popcount(int64_t b) { return std::bitset<std::numeric_limits<int64_t>::digits>(b).count(); }
    #define HAS_POPCOUNT
#endif // HAS_POPCOUNT
#ifndef HAS_POPCOUNT
    #error popcount function is not implemented for this compiler!!!
#endif // HAS_POPCOUNT

#endif // BITUTIL_H_INCLUDED
