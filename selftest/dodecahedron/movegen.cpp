#include "movegen.h"
#include "board.h"
#include "bitboard.h"
#include "cpputil.h"

inline BITBOARD find_nearest_len(const BOARD& b, int dir, int cell)
{
    for (int i = 0; i < DIR_COUNT; i++)
        if (b.all_piece & dir_val[dir][cell][i]) return i;
    return DIR_COUNT - 1;
}

inline bool inline_is_attacked(const BOARD& b, int attacker_color, int cell)
{
    #define col attacker_color
    #define find_nearest(b, dir, cell) dir_cell[dir][cell][find_nearest_len(b, dir, cell)]
    #define check_dir(var, dir) \
        if (var & dir_val[dir][cell][DIR_COUNT - 1]) \
            if (var & find_nearest(b, dir, cell)) return true;
    // Pawns
    if (b.piece_bit[col][PAWN] & pawn_attacked_by[col][cell]) return true;
    // Knights
    if (b.piece_bit[col][KNIGHT] & knight[cell]) return true;
    // Kings
    if (b.piece_bit[col][KING] & king[cell]) return true;
    // Bishops & queens
    BITBOARD bishop_queen = b.piece_bit[col][BISHOP] | b.piece_bit[col][QUEEN];
    check_dir(bishop_queen, LEFT_UP_DIR)
    check_dir(bishop_queen, LEFT_DOWN_DIR)
    check_dir(bishop_queen, RIGHT_UP_DIR)
    check_dir(bishop_queen, RIGHT_DOWN_DIR)
    // Rooks & queens
    BITBOARD rook_queen = b.piece_bit[col][ROOK] | b.piece_bit[col][QUEEN];
    check_dir(rook_queen, UP_DIR)
    check_dir(rook_queen, LEFT_DIR)
    check_dir(rook_queen, RIGHT_DIR)
    check_dir(rook_queen, DOWN_DIR)
    // Everything's OK!
    return false;
    #undef check_dir
    #undef find_nearest
    #undef col
}

bool is_attacked(const BOARD& b, int attacker_color, int cell)
{
    return inline_is_attacked(b, attacker_color, cell);
}

bool is_opponent_king_attacked(const BOARD& b)
{
    return inline_is_attacked(b, b.move_side, b.pieces[!b.move_side][KING][0]);
}

bool is_check(const BOARD& b)
{
    return inline_is_attacked(b, !b.move_side, b.pieces[b.move_side][KING][0]);
}

int gen_moves(const BOARD& b, MOVE* moves)
{
    #define push_move(_src, _dst, _promote, _flags, _double_move) \
    { \
        moves->src = _src; \
        moves->dst = _dst; \
        moves->promote = _promote; \
        moves->flags = _flags; \
        moves->double_move = _double_move; \
        moves++; cnt++; \
    }
    int cnt = 0;
    BITBOARD available_cell = ~b.col_piece[b.move_side];
    BITBOARD empty_cell = ~b.all_piece;
    BITBOARD enemy_cell = b.col_piece[!b.move_side];
    // gen for pawn
    #define push_pawn(src, dst, promotes) \
    { \
        if (promotes) \
        { \
            for (int i = KNIGHT; i <= QUEEN; i++) \
                push_move(src, dst, make_piece(b.move_side, i), FLAG_NONE, false); \
        } \
        else \
            push_move(src, dst, 0, FLAG_NONE, false); \
    }
    for (int i = 0; i < b.piece_count[b.move_side][PAWN]; i++)
    {
        int pawn_pos = b.pieces[b.move_side][PAWN][i];
        bool promotes = getbit(pawn_pos) & pawn_promote_from[b.move_side];
        // Simple moves
        if (empty_cell & pawn_single[b.move_side][pawn_pos])
        {
            push_pawn(pawn_pos, pawn_single_move[b.move_side][pawn_pos], promotes);
            if (empty_cell & pawn_double[b.move_side][pawn_pos])
                push_move(pawn_pos, pawn_double_move[b.move_side][pawn_pos], 0, FLAG_NONE, true);
        }
        // Captures
        if (enemy_cell & pawn_eat_left[b.move_side][pawn_pos])
            push_pawn(pawn_pos, pawn_eat_left_move[b.move_side][pawn_pos], promotes);
        if (enemy_cell & pawn_eat_right[b.move_side][pawn_pos])
            push_pawn(pawn_pos, pawn_eat_right_move[b.move_side][pawn_pos], promotes);
        // Enpassant
        if ((b.enpassant_line >= 0) &&
            (getbit(pawn_pos) & pawn_enpassant_src[b.enpassant_line][b.move_side]) &&
            (b.piece_bit[!b.move_side][PAWN] & pawn_enpassant[b.enpassant_line][b.move_side]))
            push_move(pawn_pos, pawn_enpassant_move[b.enpassant_line][b.move_side], 0, FLAG_ENPASSANT, false);
    }
    #undef push_pawn
    // gen for knight
    for (int i = 0; i < b.piece_count[b.move_side][KNIGHT]; i++)
    {
        int knight_pos = b.pieces[b.move_side][KNIGHT][i];
        for (int j = 0; j < knight_move_count[knight_pos]; j++)
        {
            int piece_pos = knight_moves[knight_pos][j];
            if (getbit(piece_pos) & available_cell)
                push_move(knight_pos, piece_pos, 0, FLAG_NONE, false);
        }
    }
    // gen for bishop, rook & queen
    #define gen_dir(dir, cell, i) \
        i = 0; \
        for (; dir_cell[dir][cell][i] & empty_cell; i++) \
            push_move(cell, dir_moves[dir][cell][i], 0, FLAG_NONE, false); \
        if (dir_cell[dir][cell][i] & available_cell) \
            push_move(cell, dir_moves[dir][cell][i], 0, FLAG_NONE, false);
    // bishop
    for (int i = 0; i < b.piece_count[b.move_side][BISHOP]; i++)
    {
        int cell = b.pieces[b.move_side][BISHOP][i], ctr;
        gen_dir(LEFT_UP_DIR, cell, ctr);
        gen_dir(LEFT_DOWN_DIR, cell, ctr);
        gen_dir(RIGHT_UP_DIR, cell, ctr);
        gen_dir(RIGHT_DOWN_DIR, cell, ctr);
    }
    // rook
    for (int i = 0; i < b.piece_count[b.move_side][ROOK]; i++)
    {
        int cell = b.pieces[b.move_side][ROOK][i], ctr;
        gen_dir(UP_DIR, cell, ctr);
        gen_dir(LEFT_DIR, cell, ctr);
        gen_dir(RIGHT_DIR, cell, ctr);
        gen_dir(DOWN_DIR, cell, ctr);
    }
    // queen
    for (int i = 0; i < b.piece_count[b.move_side][QUEEN]; i++)
    {
        int cell = b.pieces[b.move_side][QUEEN][i], ctr;
        gen_dir(LEFT_UP_DIR, cell, ctr);
        gen_dir(LEFT_DIR, cell, ctr);
        gen_dir(LEFT_DOWN_DIR, cell, ctr);
        gen_dir(UP_DIR, cell, ctr);
        gen_dir(DOWN_DIR, cell, ctr);
        gen_dir(RIGHT_UP_DIR, cell, ctr);
        gen_dir(RIGHT_DIR, cell, ctr);
        gen_dir(RIGHT_DOWN_DIR, cell, ctr);
    }
    #undef gen_dir
    // gen for king
    {
        int king_pos = b.pieces[b.move_side][KING][0];
        for (int j = 0; j < king_move_count[king_pos]; j++)
        {
            int piece_pos = king_moves[king_pos][j];
            if (getbit(piece_pos) & available_cell)
                push_move(king_pos, piece_pos, 0, FLAG_NONE, false);
        }
    }
    // gen for castling
    #define check_castling(kind, src_cell, tmp_cell, dst_cell, flag) \
        if (b.castling[b.move_side][kind] && \
            (!((~empty_cell) & free_castling[b.move_side][kind])) && \
            !inline_is_attacked(b, !b.move_side, src_cell) && \
            !inline_is_attacked(b, !b.move_side, tmp_cell) \
           ) \
           push_move(src_cell, dst_cell, 0, flag, false);
    int row = castling_rows[b.move_side];
    check_castling(SHORT,
                   arrpos(row, castling_src_col),
                   arrpos(row, castling_tmp_short_col),
                   arrpos(row, castling_dst_short_col),
                   FLAG_SHORT_CASTLING);
    check_castling(LONG,
                   arrpos(row, castling_src_col),
                   arrpos(row, castling_tmp_long_col),
                   arrpos(row, castling_dst_long_col),
                   FLAG_LONG_CASTLING);
    #undef check_castling
    // That's all :)
    moves->flags = FLAG_END_OF_LIST; // FLAG_END_OF_LIST must indicate the end of the move list!
    #undef push_move
    return cnt;
}
