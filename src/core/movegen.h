#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include "core/board.h"
#include "core/move.h"
#include "core/types.h"

namespace SoFCore {

// Check if the cell is attacked by pieces of color C
// Note that this function doesn't count enpassant as attack for efficiency
template <Color C>
bool isCellAttacked(const Board &b, coord_t coord);

// Checks if the move is legal AFTER it is made
// So, the typical use is to make move, then check whether it is legal, and then unmake it
bool isMoveLegal(const Board &b);

// All those functions that generate moves take arguments in the following way:
// - `board` is the current position
// - `list` is an output list in which the moves will be written
// - the return value indicates the number of moves generated
// The moves are not guaranteed to be legal, use `isMoveLegal` to check it
size_t genAllMoves(const Board &b, Move *list);
size_t genSimpleMoves(const Board &b, Move *list);
size_t genCaptures(const Board &b, Move *list);

// Returns true if this move can be returned by move generator mentioned above
// The move must be well-formed (i.e. move.isWellFormed(b.side) must return true)
// Null move is considered invalid
bool isMoveValid(const Board &b, Move move);

}  // namespace SoFCore

#endif  // MOVEGEN_H_INCLUDED
