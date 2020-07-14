#ifndef MOVES_H_INCLUDED
#define MOVES_H_INCLUDED

#include "board.h"
#include "bitboard.h"

#define FLAG_NONE 0
#define FLAG_ENPASSANT 1
#define FLAG_SHORT_CASTLING 2
#define FLAG_LONG_CASTLING 3
#define FLAG_NULL_MOVE 4
#define FLAG_END_OF_LIST 5

struct MOVE
{
    short src, dst;
    short promote;
    bool double_move;
    char flags;
};

inline MOVE impossible_move() { MOVE m; m.flags = FLAG_END_OF_LIST; return m; }
inline MOVE empty_move() { MOVE m; m.double_move = false; m.flags = FLAG_NULL_MOVE; return m; }

#define move_capture(brd, move) ((move).flags == FLAG_ENPASSANT || (brd).board[(move).dst])

inline bool moves_equal(const MOVE& a, const MOVE& b)
{
    return a.flags == b.flags && a.src == b.src && a.dst == b.dst &&
           a.promote == b.promote;
}

struct MOVE_PERSISTENCE
{
    bool was_castling[COLOR_ARRAY_SIZE][CASTLING_ARRAY_SIZE];
    PIECE was_piece;
    int was_enpassant_line;
    int was_move_counter;
};

int get_changes_list(const BOARD&, const MOVE&, int*);
void make_move(BOARD&, const MOVE&, MOVE_PERSISTENCE&);
void unmake_move(BOARD&, const MOVE&, const MOVE_PERSISTENCE&);

#endif // MOVES_H_INCLUDED
