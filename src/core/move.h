#ifndef MOVE_H_INCLUDED
#define MOVE_H_INCLUDED

#include "core/types.h"

namespace SoFCore {

enum MoveKind : uint8_t {
  Null = 0,
  Simple = 1,
  PawnDoubleMove = 2,
  Enpassant = 3,
  CastlingKingside = 4,
  CastlingQueenside = 5,
  Promote = 6
};

struct Move {
  MoveKind kind;
  coord_t src;
  coord_t dst;
  cell_t promote;

  // Checks whether the move is well-formed for moving side c. If this function returns false, then
  // it's guaranteed that the move cannot be returned by move generator. Though, it can return true
  // for some impossible moves. The purpose of this function is to be used as a pre-check for
  // isMoveValid() from movegen.h.
  bool isWellFormed(Color c) const;
  
  inline static constexpr Move null() { return Move{MoveKind::Null, 0, 0, 0}; }

  // Serialize move structure into uint32_t
  // The compiler should optimize this and just reinterpet the structure as uint32_t
  inline constexpr uint32_t intEncode() const {
    return static_cast<uint32_t>(kind) | (static_cast<uint32_t>(src) << 8) |
           (static_cast<uint32_t>(dst) << 16) | (static_cast<uint32_t>(promote) << 24);
  }
};

inline constexpr bool operator==(Move a, Move b) { return a.intEncode() == b.intEncode(); }

inline constexpr bool operator!=(Move a, Move b) { return a.intEncode() != b.intEncode(); }

// A structure to hold the information which is required to unmake the move correctly.
struct MovePersistence {
  // The order of the fields here is important to allow to save/load these fields in one mov.
  castling_t castling;
  coord_t enpassantCoord;
  uint16_t moveCounter;
  cell_t dstCell;
};

struct Board;

MovePersistence moveMake(Board &b, const Move move);
void moveUnmake(Board &b, const Move move, const MovePersistence p);

template <class Callback>
inline constexpr void iterateChangedCells(Move move, Callback callback) {
  callback(move.src);
  callback(move.dst);
  if (move.kind == MoveKind::CastlingKingside) {
    callback(move.src + 1);
    callback(move.src + 2);
    return;
  }
  if (move.kind == MoveKind::CastlingQueenside) {
    callback(move.src - 1);
    callback(move.src - 2);
    return;
  }
  if (move.kind == MoveKind::Enpassant) {
    callback((move.src < move.dst) ? (move.dst - 8) : (move.dst + 8));
    return;
  }
}

}  // namespace SoFCore

#endif  // MOVE_H_INCLUDED
