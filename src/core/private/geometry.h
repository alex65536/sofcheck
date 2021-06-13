// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_CORE_PRIVATE_GEOMETRY_INCLUDED
#define SOF_CORE_PRIVATE_GEOMETRY_INCLUDED

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

// First cell of `castlingRow`
inline constexpr coord_t castlingOffset(Color c) { return castlingRow(c) << 3; }

// Offset of the pawn of color `c` after making a single move
inline constexpr coord_t pawnMoveDelta(Color c) { return (c == Color::White) ? -8 : 8; }

}  // namespace SoFCore::Private

#endif  // SOF_CORE_PRIVATE_GEOMETRY_INCLUDED
