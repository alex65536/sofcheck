#ifndef MOVESTR_H_INCLUDED
#define MOVESTR_H_INCLUDED

#include "bitboard.h"
#include "moves.h"

void cell_to_str(int, char*);
void out_cell(int);
void move_to_str(const MOVE&, char*);
void out_move(const MOVE&);
MOVE* parse_move(MOVE*, const char*);

#endif // MOVESTR_H_INCLUDED
