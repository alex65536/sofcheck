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
