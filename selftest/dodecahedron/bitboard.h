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
#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

#include "cpputil.h"

/*  !!! WARNING !!! piece position !!!
    (P, N, B, R, Q, K) -> white pieces
    (p, n, b, r, q, k) -> black pieces

    i\j|01234567
    ------------
      0|rnbqkbnr
      1|pppppppp
      2|........
      3|........
      4|........
      5|........
      6|PPPPPPPP
      7|RNBQKBNR
*/

const int COLOR_ARRAY_SIZE = 2;
const int COL_ROW_ARRAY_SIZE = 8;
const int BOARD_ARRAY_SIZE = 64;
const int DIR_COUNT = 8;
const int DIR_ARRAY_SIZE = 8;
const int MOVE_ARRAY_SIZE = 32;
const int CASTLING_ARRAY_SIZE = 2;

typedef uint64_t BITBOARD;

#define getbit(i)    ((BITBOARD)(1) << (i))
#define bitpos(i, j) ((BITBOARD)(1) << ((i)*8 + (j)))

#define arrpos(i, j) ((i)*8 + (j))
#define cell_x(cell) ((cell) >> 3)
#define cell_y(cell) ((cell) & 7)

BITBOARD string_to_bitboard(const char*);

// Debug methods !!!
void out_bitboard(BITBOARD);

#include "bitboard_vars.h"

#endif // BITBOARD_H_INCLUDED
