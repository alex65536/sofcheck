#ifndef MOVE_H_INCLUDED
#define MOVE_H_INCLUDED

#include "core/types.h"

namespace SoFCore {

enum MoveKind : uint8_t {
  Invalid = 0,
  Simple = 1,
  Enpassant = 2,
  CastlingKingside = 3,
  CastlingQueenside = 4,
  Promote = 5
};

struct Move {
  MoveKind kind;
  coord_t src;
  coord_t dst;
  cell_t promote;

  inline static constexpr Move invalid() { return Move{MoveKind::Invalid, 0, 0, 0}; }

  // Serialize move structure into uint32_t
  // The compiler should optimize this and just reinterpet the structure as uint32_t
  inline constexpr uint32_t intEncode() const {
    return static_cast<uint32_t>(kind) | (static_cast<uint32_t>(src) << 8) |
           (static_cast<uint32_t>(dst) << 16) | (static_cast<uint32_t>(promote) << 24);
  }
};

inline constexpr bool operator==(Move a, Move b) { return a.intEncode() == b.intEncode(); }

inline constexpr bool operator!=(Move a, Move b) { return a.intEncode() != b.intEncode(); }

template<class Callback>
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
