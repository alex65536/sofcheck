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
#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "bitboard.h"
#include "board_defines.h"

typedef int PIECE;
#define make_piece(color, kind) (((color) << 3) | (kind))
#define get_color(piece) ((piece) >> 3)
#define get_kind(piece)  ((piece) & 7)
#define is_color(piece, color) (get_color(piece) == color)

const int PIECE_ARRAY_SIZE = 7;
const int COLOR_PIECE_ARRAY_SIZE = 16;
const int MAX_PIECE_COUNT = 16;

// Enpassant consts
const int enpassant_row[COLOR_ARRAY_SIZE] = {3, 4};

// Castling consts
const int castling_rows[COLOR_ARRAY_SIZE] = {7, 0};
const int castling_rook_cols[CASTLING_ARRAY_SIZE] = {7, 0};
const int castling_king_col = 4;

const int castling_src_col = 4;

const int castling_tmp_short_col = 5,
          castling_tmp_long_col  = 3;

const int castling_dst_short_col = 6,
          castling_dst_long_col  = 2;

const int castling_rook_short_col = 7,
          castling_rook_long_col  = 0;

struct BOARD
{
    // Base fields
    PIECE board[BOARD_ARRAY_SIZE];
    bool castling[COLOR_ARRAY_SIZE][CASTLING_ARRAY_SIZE];
    int move_side;
    int enpassant_line;
    int move_counter;
    // Additional fields
    int piece_count[COLOR_ARRAY_SIZE][PIECE_ARRAY_SIZE];
    int pieces[COLOR_ARRAY_SIZE][PIECE_ARRAY_SIZE][MAX_PIECE_COUNT];
    int list_pos[BOARD_ARRAY_SIZE];
    BITBOARD all_piece;
    BITBOARD col_piece[COLOR_ARRAY_SIZE];
    BITBOARD piece_bit[COLOR_ARRAY_SIZE][PIECE_ARRAY_SIZE];
};

void out_board(const BOARD&);
void recalc_board(BOARD&);
bool load_from_fen(BOARD&, const char*);
void save_to_fen(const BOARD&, char*);
bool validate_board(BOARD&);
BOARD start_position();
BOARD clear_board();
bool board_ok(const BOARD&);

#endif // BOARD_H_INCLUDED
