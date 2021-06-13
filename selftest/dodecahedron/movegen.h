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
