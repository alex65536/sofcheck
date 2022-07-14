// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DODECAHEDRON_INTF_H_INCLUDED
#define DODECAHEDRON_INTF_H_INCLUDED

#include "board.h"
#include "moves.h"
#include "movestr.h"
#include "movegen.h"

namespace ChessIntf {

// This one doesn't support self-tests
// #define CHESS_INTF_HAS_SELF_TESTS

using Board = BOARD;
using Move = MOVE;
using MovePersistence = MOVE_PERSISTENCE;

constexpr int MOVES_MAX = 240;

struct MoveList {
    Move moves[MOVES_MAX];
    int count;
};

inline const char *getImplName() {
    return "Dodecahedron";
}

inline const Move &getMove(const MoveList &lst, int idx) {
    return lst.moves[idx];
}

inline int getMoveCount(const MoveList &lst) {
    return lst.count;
}

inline void init() {}

inline Board boardFromFen(const char *fen) {
    Board board;
    load_from_fen(board, fen);
    return board;
}

inline MovePersistence makeMove(Board &board, const Move &move) {
    MovePersistence p;
    make_move(board, move, p);
    return p;
}

inline void unmakeMove(Board &board, const Move &move, MovePersistence &p) {
    unmake_move(board, move, p);
}

inline void moveStr(const Board &, const Move &move, char *str) {
    move_to_str(move, str);
}

inline MoveList generateMoves(const Board &board) {
    MoveList moves;
    moves.count = gen_moves(board, moves.moves);
    return moves;
}

inline bool isAttacked(const Board &board, bool isWhite, char cx, char cy) {
    return is_attacked(board, isWhite ? WHITE : BLACK, arrpos('8' - cy, cx - 'a'));
}

inline bool isLastMoveLegal(const Board &board) {
    return !is_opponent_king_attacked(board);
}

}

#endif

