#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include "board.h"
#include "moves.h"

bool is_attacked(const BOARD&, int, int); // Without enpassant!
bool is_opponent_king_attacked(const BOARD&);
bool is_check(const BOARD&);

// TODO: Make a generator only for captures!
int gen_moves(const BOARD&, MOVE*);


#endif // MOVEGEN_H_INCLUDED
