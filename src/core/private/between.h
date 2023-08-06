// This file is part of SoFCheck
//
// Copyright (c) 2023 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_CORE_PRIVATE_BETWEEN_INCLUDED
#define SOF_CORE_PRIVATE_BETWEEN_INCLUDED

#include <algorithm>

#include "core/private/between_consts.h"
#include "core/types.h"

namespace SoFCore::Private {

// If `src` and `dst` are both on the same line (either horizontal, vertical or diagonal), returns
// all the cells strictly between them. Otherwise, returns an empty bitboard.
inline bitboard_t between(coord_t src, coord_t dst) {
  if (src > dst) {
    std::swap(src, dst);
  }
  const auto bbDst = coordToBitboard(dst);
  if (const auto bishopGt = BISHOP_GT[src]; bishopGt & bbDst) {
    return bishopGt & BISHOP_LT[dst];
  }
  if (const auto rookGt = ROOK_GT[src]; rookGt & bbDst) {
    return rookGt & ROOK_LT[dst];
  }
  return 0;
}

}  // namespace SoFCore::Private

#endif  // SOF_CORE_PRIVATE_BITBOARD_INCLUDED
