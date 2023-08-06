// This file is part of SoFCheck
//
// Copyright (c) 2020-2023 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOFCHECK_INTF_H_INCLUDED
#define SOFCHECK_INTF_H_INCLUDED

#include <optional>

#include "core/board.h"
#include "core/init.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "core/test/selftest.h"
#include "util/misc.h"

namespace ChessIntf {

#define CHESS_INTF_HAS_SELF_TESTS

using Board = SoFCore::Board;
using Move = SoFCore::Move;
using MovePersistence = std::optional<SoFCore::MovePersistence>;

constexpr int MOVES_MAX = SoFCore::BUFSZ_MOVES;

struct MoveList {
  Move moves[MOVES_MAX];
  int count;
};

inline const char *getImplName() { return "SoFCheck"; }

inline void selfTest(Board board) { SoFCore::Test::runSelfTest(board); }

inline void init() { SoFCore::init(); }

inline const Move &getMove(const MoveList &lst, int idx) { return lst.moves[idx]; }

inline int getMoveCount(const MoveList &lst) { return lst.count; }

inline Board boardFromFen(const char *fen) {
  Board board;  // NOLINT: uninitialized
  SoFCore::FenParseResult result = board.setFromFen(fen);
  if (result != SoFCore::FenParseResult::Ok) {
    SoFUtil::panic("the given FEN is invalid");
  }
  return board;
}

inline MovePersistence tryMakeMove(Board &board, const Move &move) {
  auto p = SoFCore::moveMake(board, move);
  if (!isMoveLegal(board)) {
    SoFCore::moveUnmake(board, move, p);
    return std::nullopt;
  }
  return p;
}

inline bool isMoveMade(const MovePersistence& p) {
  return p.has_value();
}

inline void unmakeMove(Board &board, const Move &move, MovePersistence &p) {
  SoFCore::moveUnmake(board, move, *p);
}

inline void moveStr(const Board &, const Move &mv, char *str) { SoFCore::moveToStr(mv, str); }

inline MoveList generateMoves(const Board &board) {
  MoveList moves;
  SoFCore::MoveGen gen(board);
  moves.count = static_cast<int>(gen.genAllMoves(moves.moves));
  return moves;
}

inline bool isAttacked(const Board &board, bool isWhite, char cy, char cx) {
  using namespace SoFCore;
  coord_t coord = charsToCoord(cy, cx);
  if (isWhite) {
    return isCellAttacked<Color::White>(board, coord);
  } else {
    return isCellAttacked<Color::Black>(board, coord);
  }
}

inline bool isInCheck(const Board &board) { return isCheck(board); }

}  // namespace ChessIntf

#endif
