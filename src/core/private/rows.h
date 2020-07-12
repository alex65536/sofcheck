#ifndef SOF_CORE_PRIVATE_ROWS_INCLUDED
#define SOF_CORE_PRIVATE_ROWS_INCLUDED

#include "core/types.h"

namespace SoFCore::Private {

// Row from which the pawn can make enapssant
inline constexpr subcoord_t enpassantSrcRow(Color c) { return (c == Color::White) ? 3 : 4; }

// Row in which the pawn goes after enapssant capture
inline constexpr subcoord_t enpassantDstRow(Color c) { return (c == Color::White) ? 2 : 5; }

// Row from which the pawn can promote
inline constexpr subcoord_t promoteSrcRow(Color c) { return (c == Color::White) ? 1 : 6; }

// Row in which the pawn goes after promote
inline constexpr subcoord_t promoteDstRow(Color c) { return (c == Color::White) ? 0 : 7; }

// Row from which the pawn can make double move
inline constexpr subcoord_t doubleMoveSrcRow(Color c) { return (c == Color::White) ? 6 : 1; }

// Row in which the pawn goes after double move
inline constexpr subcoord_t doubleMoveDstRow(Color c) { return (c == Color::White) ? 4 : 3; }

// Row in which the castling is performed
inline constexpr subcoord_t castlingRow(Color c) { return (c == Color::White) ? 7 : 0; }

}  // namespace SoFCore::Private

#endif  // SOF_CORE_PRIVATE_ROWS_INCLUDED
