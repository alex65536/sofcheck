#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include "core/board.h"
#include "core/types.h"

namespace SoFCore {

// Check if the cell is attacked by pieces of color C
// Note that this function doesn't count enpassant as attack for efficiency
template<Color C>
bool isCellAttacked_simple(const Board &b, coord_t coord);

template<Color C>
bool isCellAttacked_simpleArrs(const Board &b, coord_t coord);

template<Color C>
bool isCellAttacked_sse(const Board &b, coord_t coord);

template<Color C>
bool isCellAttacked_avx(const Board &b, coord_t coord);

}

#endif // MOVEGEN_H_INCLUDED
