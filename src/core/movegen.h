// This file is part of SoFCheck
//
// Copyright (c) 2020-2021, 2023 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_CORE_MOVEGEN_INCLUDED
#define SOF_CORE_MOVEGEN_INCLUDED

#include <cstddef>

#include "core/board.h"
#include "core/move.h"
#include "core/types.h"

namespace SoFCore {

// Checks if the cell is attacked by any of the pieces of color `C`
//
// For optimization purposes, enpassant captures are not considered by this function, as the primary
// purpose of this function is to check for king attacks (e. g. in `wasMoveLegal()`)
template <Color C>
bool isCellAttacked(const Board &b, coord_t coord);

inline bool isCellAttacked(const Board &b, const coord_t coord, const Color c) {
  return (c == Color::White) ? isCellAttacked<Color::White>(b, coord)
                             : isCellAttacked<Color::Black>(b, coord);
}

// Returns the set of pieces of color `C` which attack the given cell
//
// For optimization purposes, enpassant captures are not considered by this function, as the primary
// purpose of this function is to check for king attacks (e. g. in `wasMoveLegal()`)
template <Color C>
bitboard_t cellAttackers(const Board &b, coord_t coord);

inline bitboard_t cellAttackers(const Board &b, const coord_t coord, const Color c) {
  return (c == Color::White) ? cellAttackers<Color::White>(b, coord)
                             : cellAttackers<Color::Black>(b, coord);
}

// Returns `true` if the last move applied to the board `b` was legal. Note that it doesn't mean
// that you can apply any illegal moves to the board, the applied move must be still pseudo-legal.
//
// So, the typical use of this function is to make a pseudo-legal move, then check whether it is
// legal, and then unmake it if it's illegal.
bool wasMoveLegal(const Board &b);

// Returns `true` is the king of the moving side is currenly under check
bool isCheck(const Board &b);

// Move generator
class MoveGen {
public:
  // Creates a move generator for the board `b`. The provided board must not be changed from the
  // outside, though making some moves and reverting them between the calls is allowed.
  explicit MoveGen(const Board &b);

  // Returns a pointer to the board, for which the moves are being generated.
  const Board &board() const { return b_; }

  // All these functions generate all the legal moves plus some pseudo-legal moves. Thus, after
  // applying some of the generated moves, the king may become under check, but otherwise the moves
  // are valid according to the rules of chess. The moves are written into `list`, and the return
  // value indicates the number of moves generated.
  //
  // To generate only legal moves you can use `isMoveLegal()` or `wasMoveLegal()` function
  size_t genAllMoves(Move *list) const;
  size_t genSimpleMoves(Move *list) const;
  size_t genSimpleMovesNoPromote(Move *list) const;
  size_t genSimplePromotes(Move *list) const;
  size_t genCaptures(Move *list) const;

private:
  enum class CheckKind : int8_t {
    None = 0,
    Single = 1,
    Double = 2,
  };

  const Board &b_;
  bitboard_t checkMask_ = 0;
  CheckKind check_;
  Color side_;

  template <Color, bool, bool, bool>
  size_t genImpl(Move *list) const;

  template <Color>
  size_t genSimplePromotesImpl(Move *list) const;
};

// Upper bound for total number of pseudo-legal moves in any valid position. You can use it as a
// buffer size for `MoveGen::genAllMoves()`.
constexpr size_t BUFSZ_MOVES = 300;

// Upper bound for total number of pseudo-legal captures in any valid position. You can use it as a
// buffer size for `MoveGen::genCaptures()`.
constexpr size_t BUFSZ_CAPTURES = 128;

// Upper bound for total number of pseudo-legal simple promotes in any valid position. You can use
// it as a buffer size for `MoveGen::genSimplePromotes()`.
constexpr size_t BUFSZ_SIMPLE_PROMOTES = 32;

// Returns `true` if the move `move` is pseudo-legal
//
// The move must be well-formed (i.e. `move.isWellFormed(b.side)` must return `true`), otherwise the
// behavior is undefined. Null moves are considered invalid by this function, as they cannot be
// returned by `genAllMoves()`.
//
// To check for legality you can use `isMoveLegal()` or `wasMoveLegal()` function
bool isMoveValid(const Board &b, Move move);

// Returns `true` if the move `move` is legal
//
// The move must be pseudo-legal (i.e. `move.isWellFormed(b.side) && isMoveValid(b, move)` must
// return `true`), otherwise the behavior is undefined.
//
// As a special exception, null moves are considered legal by this function, but only if there is no
// check.
bool isMoveLegal(const Board &b, Move move);

// Returns `true` if the move is capture
inline constexpr bool isMoveCapture(const Board &b, const Move move) {
  return b.cells[move.dst] != EMPTY_CELL || move.kind == MoveKind::Enpassant;
}

}  // namespace SoFCore

#endif  // SOF_CORE_MOVEGEN_INCLUDED
