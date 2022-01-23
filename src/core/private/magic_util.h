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

#ifndef SOF_CORE_PRIVATE_MAGIC_UTIL_INCLUDED
#define SOF_CORE_PRIVATE_MAGIC_UTIL_INCLUDED

#include "core/bitboard.h"
#include "core/private/bitboard.h"
#include "core/types.h"
#include "util/bit.h"

namespace SoFCore::Private {

inline constexpr bitboard_t buildMagicRookMask(const coord_t c) {
  const subcoord_t x = coordX(c);
  const subcoord_t y = coordY(c);
  return ((BB_ROW[x] & ~BB_ROW_FRAME) | (BB_COL[y] & ~BB_COL_FRAME)) & ~coordToBitboard(c);
}

inline constexpr bitboard_t buildMagicRookPostMask(const coord_t c) {
  const subcoord_t x = coordX(c);
  const subcoord_t y = coordY(c);
  return BB_ROW[x] ^ BB_COL[y];
}

inline constexpr bitboard_t buildMagicBishopMask(const coord_t c) {
  return (BB_DIAG1[coordDiag1(c)] ^ BB_DIAG2[coordDiag2(c)]) & ~BB_DIAG_FRAME;
}

inline constexpr bitboard_t buildMagicBishopPostMask(const coord_t c) {
  return BB_DIAG1[coordDiag1(c)] ^ BB_DIAG2[coordDiag2(c)];
}

enum class MagicType { Rook, Bishop };

template <MagicType M>
inline constexpr bitboard_t buildMagicMask(const coord_t c) {
  return (M == MagicType::Rook) ? buildMagicRookMask(c) : buildMagicBishopMask(c);
}

template <MagicType M>
inline constexpr bitboard_t buildMagicPostMask(const coord_t c) {
  return (M == MagicType::Rook) ? buildMagicRookPostMask(c) : buildMagicBishopPostMask(c);
}

template <MagicType M>
inline constexpr size_t getMagicMaskBitSize(const coord_t c) {
  return SoFUtil::popcount(buildMagicMask<M>(c));
}

}  // namespace SoFCore::Private

#endif  // SOF_CORE_PRIVATE_MAGIC_UTIL_INCLUDED
