#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include "core/board.h"
#include "core/types.h"
#include "core/move.h"

namespace SoFCore {

// Check if the cell is attacked by pieces of color C
// Note that this function doesn't count enpassant as attack for efficiency
template<Color C>
bool isCellAttacked(const Board &b, coord_t coord);

}

#endif // MOVEGEN_H_INCLUDED
