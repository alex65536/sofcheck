// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

#include "core/move_parser.h"

#include <algorithm>

#include "core/board.h"
#include "core/private/geometry.h"
#include "core/strutil.h"
#include "util/misc.h"

namespace SoFCore {

ParsedMove ParsedMove::fromStr(const char *first, const char *last) {
  if (last != first + 4 && last != first + 5) {
    return INVALID_PARSED_MOVE;
  }
  // Parse null move (it is written as "0000")
  if (SOF_UNLIKELY(last == first + 4 && std::equal(first, last, "0000"))) {
    return ParsedMove{PromotePiece::None, 0, 0, 0};
  }
  if (!isYCharValid(first[0]) || !isXCharValid(first[1]) || !isYCharValid(first[2]) ||
      !isXCharValid(first[3])) {
    return INVALID_PARSED_MOVE;
  }
  ParsedMove result{PromotePiece::None, charsToCoord(first[0], first[1]),
                    charsToCoord(first[2], first[3]), 0};
  if (last == first + 5) {
    switch (first[4]) {
      case 'n':
        result.promote = PromotePiece::Knight;
        break;
      case 'b':
        result.promote = PromotePiece::Bishop;
        break;
      case 'r':
        result.promote = PromotePiece::Rook;
        break;
      case 'q':
        result.promote = PromotePiece::Queen;
        break;
      default:
        return INVALID_PARSED_MOVE;
    }
  }
  return result;
}

template <Color C>
inline static Move moveFromParsedImpl(const ParsedMove p, const Board &board) {
  // Convert invalid ParsedMove into invalid Move
  if (SOF_UNLIKELY(p == INVALID_PARSED_MOVE)) {
    return Move::invalid();
  }

  // Convert null move
  constexpr ParsedMove nullParsedMove{PromotePiece::None, 0, 0, 0};
  if (SOF_UNLIKELY(p == nullParsedMove)) {
    return Move::null();
  }

  // Convert promote
  if (p.promote != PromotePiece::None) {
    const auto kind = static_cast<MoveKind>(static_cast<int8_t>(p.promote) -
                                            static_cast<int8_t>(PromotePiece::Knight) +
                                            static_cast<int8_t>(MoveKind::PromoteKnight));
    return Move{kind, p.src, p.dst, 0};
  }

  Move result{MoveKind::Simple, p.src, p.dst, 0};

  // Convert unusual pawn moves (enpassant and double move)
  if (board.cells[p.src] == makeCell(C, Piece::Pawn)) {
    if (coordX(p.src) == Private::doubleMoveSrcRow(C) &&
        coordX(p.dst) == Private::doubleMoveDstRow(C)) {
      result.kind = MoveKind::PawnDoubleMove;
      return result;
    }
    if (coordY(p.src) != coordY(p.dst) && board.cells[p.dst] == EMPTY_CELL) {
      result.kind = MoveKind::Enpassant;
      return result;
    }
  }

  // Convert castling
  if (board.cells[p.src] == makeCell(C, Piece::King)) {
    constexpr subcoord_t x = Private::castlingRow(C);
    if (p.src == makeCoord(x, 4) && p.dst == makeCoord(x, 6)) {
      result.kind = MoveKind::CastlingKingside;
      return result;
    }
    if (p.src == makeCoord(x, 4) && p.dst == makeCoord(x, 2)) {
      result.kind = MoveKind::CastlingQueenside;
      return result;
    }
  }

  return result;
}

Move moveFromParsed(const ParsedMove parsedMove, const Board &board) {
  return (board.side == Color::White) ? moveFromParsedImpl<Color::White>(parsedMove, board)
                                      : moveFromParsedImpl<Color::Black>(parsedMove, board);
}

}  // namespace SoFCore
