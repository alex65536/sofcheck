#ifndef BITBOARD_VARS_H_INCLUDED
#define BITBOARD_VARS_H_INCLUDED

#include "board_defines.h"

#ifdef NO_USE_EXTERN
    #define __extern
#else
    #define __extern extern
#endif

#define LEFT_DOWN_DIR  0
#define DOWN_DIR       1
#define RIGHT_DOWN_DIR 2
#define LEFT_DIR       3
#define RIGHT_DIR      4
#define LEFT_UP_DIR    5
#define UP_DIR         6
#define RIGHT_UP_DIR   7

/* Bitboards */
// Pawns
__extern BITBOARD pawn_single[COLOR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
__extern BITBOARD pawn_double[COLOR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
__extern BITBOARD pawn_eat_left[COLOR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
__extern BITBOARD pawn_eat_right[COLOR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
__extern BITBOARD pawn_attacked_by [COLOR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
__extern BITBOARD pawn_promote_from[COLOR_ARRAY_SIZE];
__extern BITBOARD pawn_enpassant_src[COL_ROW_ARRAY_SIZE][COLOR_ARRAY_SIZE];
__extern BITBOARD pawn_enpassant[COL_ROW_ARRAY_SIZE][COLOR_ARRAY_SIZE];
// Knights
__extern BITBOARD knight[BOARD_ARRAY_SIZE];
// Kings
__extern BITBOARD king[BOARD_ARRAY_SIZE];
// Bishops, rooks & queens
__extern BITBOARD dir_val[DIR_ARRAY_SIZE][BOARD_ARRAY_SIZE][DIR_COUNT];
__extern BITBOARD dir_cell[DIR_ARRAY_SIZE][BOARD_ARRAY_SIZE][DIR_COUNT];
// Castling
__extern BITBOARD free_castling[COLOR_ARRAY_SIZE][CASTLING_ARRAY_SIZE];

/* Possible moves */
// Knights
__extern int knight_moves[BOARD_ARRAY_SIZE][MOVE_ARRAY_SIZE];
__extern int knight_move_count[BOARD_ARRAY_SIZE];
// Kings
__extern int king_moves[BOARD_ARRAY_SIZE][MOVE_ARRAY_SIZE];
__extern int king_move_count[BOARD_ARRAY_SIZE];
// Bishops, rooks & queens
__extern int dir_moves[DIR_ARRAY_SIZE][BOARD_ARRAY_SIZE][MOVE_ARRAY_SIZE];
__extern int dir_move_count[DIR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
// Pawns
__extern int pawn_single_move[COLOR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
__extern int pawn_double_move[COLOR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
__extern int pawn_eat_left_move[COLOR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
__extern int pawn_eat_right_move[COLOR_ARRAY_SIZE][BOARD_ARRAY_SIZE];
__extern int pawn_enpassant_move[COL_ROW_ARRAY_SIZE][COLOR_ARRAY_SIZE];

#undef __extern

#endif // BITBOARD_VARS_H_INCLUDED
