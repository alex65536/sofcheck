#ifndef SOF_CORE_MOVE_INCLUDED
#define SOF_CORE_MOVE_INCLUDED

#include "core/types.h"

namespace SoFCore {

enum MoveKind : int8_t {
  Null = 0,
  Simple = 1,
  PawnDoubleMove = 2,
  Enpassant = 3,
  CastlingKingside = 4,
  CastlingQueenside = 5,
  PromoteKnight = 6,
  PromoteBishop = 7,
  PromoteRook = 8,
  PromoteQueen = 9,
  Invalid = 10
};

// Returns `true` if the move is a pawn promote
inline constexpr bool isMoveKindPromote(const MoveKind kind) {
  return kind == MoveKind::PromoteKnight || kind == MoveKind::PromoteBishop ||
         kind == MoveKind::PromoteRook || kind == MoveKind::PromoteQueen;
}

// Returns the promote piece if `isMoveKindPromote()` is `true`. Otherwise, the behaviour is
// undefined
inline constexpr Piece moveKindPromotePiece(const MoveKind kind) {
  return static_cast<Piece>(static_cast<int8_t>(kind) -
                            static_cast<int8_t>(MoveKind::PromoteKnight) +
                            static_cast<int8_t>(Piece::Knight));
}

struct Move {
  MoveKind kind;
  coord_t src;
  coord_t dst;
  uint8_t tag;  // Can be used to store additional info about the move. Zeroed by default

  // Checks if the move is well-formed for moving side `c`.
  //
  // If this function returns `false`, then it's guaranteed that the move cannot be returned by move
  // generator on any position with move side `c`. Though, it can return `true` for some impossible
  // moves. The purpose of this function is to be used as a pre-check for `isMoveValid()`. Also
  // please note that null move is considered well-formed
  bool isWellFormed(Color c) const;

  inline static constexpr Move null() { return Move{MoveKind::Null, 0, 0, 0}; }
  inline static constexpr Move invalid() { return Move{MoveKind::Invalid, 0, 0, 0}; }

  // Serializes the move structure into `uint32_t`
  //
  // The compiler should optimize this and just reinterpet the structure as `uint32_t` for
  // little-endian architectures. The engine is not optimized for big-endian now, so there is no
  // fast implementation for such architectures at the moment.
  inline constexpr uint32_t asUint() const {
    const auto uintKind = static_cast<uint8_t>(kind);
    const auto uintSrc = static_cast<uint8_t>(src);
    const auto uintDst = static_cast<uint8_t>(dst);
    return static_cast<uint32_t>(uintKind) | (static_cast<uint32_t>(uintSrc) << 8) |
           (static_cast<uint32_t>(uintDst) << 16) | (static_cast<uint32_t>(tag) << 24);
  }
};

inline constexpr bool operator==(Move a, Move b) { return a.asUint() == b.asUint(); }

inline constexpr bool operator!=(Move a, Move b) { return a.asUint() != b.asUint(); }

// A structure to hold the information which is required to unmake the move correctly.
struct MovePersistence {
  board_hash_t hash;
  // The order of the fields here is important, as it matches with the similar fields in `Board`.
  // This allows us to save and load these fields in one `mov` instruction or something like this
  castling_t castling;
  coord_t enpassantCoord;
  uint16_t moveCounter;
  cell_t dstCell;
  // Padding to make structure size equal to 16 bytes. Will be set to zero
  uint8_t padding1;
  uint16_t padding2;
};

struct Board;

// Applies move `move` to the board `b`. The move must be pseudo-legal, i.e. must be in the list
// generated from current position using `genAllMoves()`. Otherwise, the behaviour is undefined.
//
// As a special exception, making null moves is also allowed. If you pass a null move to this
// function, it will just flip the current move side and update the move counter and move number.
//
// The return value is useful to undo this operation using `moveUnmake()`.
MovePersistence moveMake(Board &b, Move move);

// Undoes the operation made by `moveMake()`. The parameter `p` must the value returned from
// corresponding `moveMake()`.
//
// Note that the behaviour of this function is undefined if move `move` is not the last move applied
// to the board `b`, and `p` is not a value returned by `moveMake()` that applied this move. By the
// way, it is valid to make and unmake other moves between `moveMake()/moveUnmake()` pair, provided
// that the board after `moveMake()` and before corresponding `moveUnmake()` is the same. For
// example, this code is valid:
//
// MovePersistence p1 = moveMake(b, move1);
// MovePersistence p2 = moveMake(b, move2);
// moveUnmake(b, move2, p2);
// moveUnmake(b, move1, p1);
void moveUnmake(Board &b, Move move, MovePersistence p);

// Calls `callback` for each cell that will be changed by the move `move`.
template <typename Callback>
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

#endif  // SOF_CORE_MOVE_INCLUDED
