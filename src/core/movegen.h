#ifndef SOF_CORE_MOVEGEN_INCLUDED
#define SOF_CORE_MOVEGEN_INCLUDED

#include "core/board.h"
#include "core/move.h"
#include "core/types.h"

namespace SoFCore {

// Checks if the cell is attacked by any of the pieces of color `C`
//
// For optimization purposes, enpassant captures are not considered by this function, as the primary
// purpose of this function is to check for king attacks  (e. g. in `isMoveLegal()`)
template <Color C>
bool isCellAttacked(const Board &b, coord_t coord);

// Returns `true` if the last move applied to the board `b` was legal. Note that it doesn't mean
// that you can apply any illegal moves to the board, the applied move must be still pseudo-legal.
//
// So, the typical use of this function is to make a pseudo-legal move, then check whether it is
// legal, and then unmake it if it's illegal.
bool isMoveLegal(const Board &b);

// Returns `true` is the king of the moving side is currenly under check
bool isCheck(const Board &b);

// All these functions generate pseudo-legal moves (i.e. all the moves that are legal by chess rules
// if we ignore the rule that the king must not be under check). The arguments are passed in the
// following way:
//
// - `board` is the current position
// - `list` is an output list in which the moves will be written
// - the return value indicates the number of moves generated
//
// To generate only legal moves you can use `isMoveLegal()` function
size_t genAllMoves(const Board &b, Move *list);
size_t genSimpleMoves(const Board &b, Move *list);
size_t genCaptures(const Board &b, Move *list);

// Returns `true` if the move `move` can be returned by `genAllMoves()`
//
// The move must be well-formed (i.e. `move.isWellFormed(b.side)` must return `true`), otherwise the
// behavior is undefined. Null moves are considered invalid by this function, as they cannot be
// returned by `genAllMoves()`.
bool isMoveValid(const Board &b, Move move);

}  // namespace SoFCore

#endif  // SOF_CORE_MOVEGEN_INCLUDED
