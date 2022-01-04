// This file is part of SoFCheck
//
// Copyright (c) 2022 Alexander Kernozhitsky and SoFCheck contributors
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

#include "eval/private/pawn_cache.h"

#include <algorithm>

#include "core/private/zobrist.h"  // FIXME : refactor

namespace SoFEval::Private {

using SoFCore::Board;
using SoFCore::board_hash_t;
using SoFCore::cell_t;
using SoFCore::coord_t;
using SoFCore::Move;
using SoFCore::MoveKind;
using SoFCore::Piece;

board_hash_t pawnHashInit(const Board &b) {
  board_hash_t hash = 0;
  for (coord_t i = 0; i < 64; ++i) {
    const cell_t cell = b.cells[i];
    if (cell == SoFCore::EMPTY_CELL || SoFCore::cellPiece(cell) != Piece::Pawn) {
      continue;
    }
    hash ^= SoFCore::Private::g_zobristPieces[cell][i];
  }
  return hash;
}

board_hash_t pawnHashUpdate(const Board &b, board_hash_t hash, const Move move) {
  if (move.kind == MoveKind::Null || move.kind == MoveKind::CastlingQueenside ||
      move.kind == MoveKind::CastlingKingside) {
    return hash;
  }
  const SoFCore::Color color = b.side;
  const cell_t srcCell = b.cells[move.src];
  const cell_t ourPawn = makeCell(color, Piece::Pawn);
  const cell_t dstCell = b.cells[move.dst];
  const cell_t enemyPawn = makeCell(invert(color), Piece::Pawn);
  if (srcCell != ourPawn && dstCell != enemyPawn) {
    return hash;
  }
  if (isMoveKindPromote(move.kind)) {
    hash ^= SoFCore::Private::g_zobristPieces[ourPawn][move.src];
    return hash;
  }
  if (srcCell == ourPawn) {
    hash ^= SoFCore::Private::g_zobristPieces[ourPawn][move.src];
    hash ^= SoFCore::Private::g_zobristPieces[ourPawn][move.dst];
  }
  if (dstCell == enemyPawn) {
    hash ^= SoFCore::Private::g_zobristPieces[enemyPawn][move.dst];
  }
  if (move.kind == MoveKind::Enpassant) {
    const coord_t pawnPos = enpassantPawnPos(color, move.dst);
    hash ^= SoFCore::Private::g_zobristPieces[enemyPawn][pawnPos];
  }
  return hash;
}

PawnCache<score_t>::PawnCache() { std::fill(entries_, entries_ + CACHE_SIZE, Entry::invalid()); }

}  // namespace SoFEval::Private
