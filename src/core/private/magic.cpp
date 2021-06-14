// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

#include "core/private/magic.h"

#include <algorithm>

#include "core/private/magic_util.h"
#include "util/bit.h"

namespace SoFCore::Private {

MagicEntry g_magicRook[64];
MagicEntry g_magicBishop[64];

static bitboard_t g_rookLookup[65536];
static bitboard_t g_bishopLookup[1792];

// Determine pointers for shared rook and bishop arrays
// For rooks we share two cells (one entry for both `c1` and `c2`)
// For bishops the number of shared cells is equal to four
// To find more details, see https://www.chessprogramming.org/Magic_Bitboards#Sharing_Attacks
template <MagicType M>
inline static void initOffsets(size_t bases[]) {
  size_t count = 0;
  if constexpr (M == MagicType::Rook) {
    for (coord_t c1 = 0; c1 < 64; ++c1) {
      const coord_t c2 = c1 ^ 9;
      if (c1 > c2) {
        continue;
      }
      const size_t maxLen = std::max(getMagicMaskBitSize<M>(c1), getMagicMaskBitSize<M>(c2));
      const size_t add = static_cast<size_t>(1U) << maxLen;
      bases[c1] = count;
      bases[c2] = count;
      count += add;
    }
  } else {
    const coord_t starts[16] = {0, 1, 32, 33, 2, 10, 18, 26, 34, 42, 50, 58, 6, 7, 38, 39};
    const coord_t offsets[16] = {8, 8, 8, 8, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 8, 8};
    for (size_t idx = 0; idx < 16; ++idx) {
      // We consider 16 groups of shared bishop cells. A single group contains four cells with
      // coordinates `c + i * offs` for all `i = 0..3`.
      const coord_t c = starts[idx];
      const coord_t offs = offsets[idx];
      const size_t maxLen = std::max(
          std::max(getMagicMaskBitSize<M>(c + 0 * offs), getMagicMaskBitSize<M>(c + 1 * offs)),
          std::max(getMagicMaskBitSize<M>(c + 2 * offs), getMagicMaskBitSize<M>(c + 3 * offs)));
      const size_t add = static_cast<size_t>(1U) << maxLen;
      for (coord_t i = 0; i < 4; ++i) {
        bases[c + i * offs] = count;
      }
      count += add;
    }
  }
}

template <MagicType M>
static void initMagic() {
  MagicEntry *magicEntries = (M == MagicType::Rook) ? g_magicRook : g_magicBishop;
  bitboard_t *lookup = (M == MagicType::Rook ? g_rookLookup : g_bishopLookup);

  // First, generate attack masks
  for (coord_t c = 0; c < 64; ++c) {
    magicEntries[c].mask = buildMagicMask<M>(c);
    magicEntries[c].postMask = buildMagicPostMask<M>(c);
  }

  // Fill lookup table offsets for each cell
  size_t offsets[64];
  initOffsets<M>(offsets);
  for (coord_t c = 0; c < 64; ++c) {
    magicEntries[c].lookup = lookup + offsets[c];
  }

  // Fill lookup table
  for (coord_t c = 0; c < 64; ++c) {
    const bitboard_t mask = magicEntries[c].mask;
    const size_t submaskCnt = static_cast<size_t>(1U) << SoFUtil::popcount(mask);
    const size_t offset = offsets[c];
    constexpr static const int8_t DX_BISHOP[4] = {-1, 1, -1, 1};
    constexpr static const int8_t DY_BISHOP[4] = {-1, -1, 1, 1};
    constexpr static const int8_t DX_ROOK[4] = {-1, 1, 0, 0};
    constexpr static const int8_t DY_ROOK[4] = {0, 0, -1, 1};
    constexpr auto *dx = (M == MagicType::Rook) ? DX_ROOK : DX_BISHOP;
    constexpr auto *dy = (M == MagicType::Rook) ? DY_ROOK : DY_BISHOP;
    for (size_t submask = 0; submask < submaskCnt; ++submask) {
      const bitboard_t occupied = SoFUtil::depositBits(submask, mask);
#ifdef USE_BMI2
      const size_t pos = submask;
#else
      constexpr auto *magics = (M == MagicType::Rook) ? ROOK_MAGICS : BISHOP_MAGICS;
      constexpr auto *shifts = (M == MagicType::Rook) ? ROOK_SHIFTS : BISHOP_SHIFTS;
      const auto pos = static_cast<size_t>((occupied * magics[c]) >> shifts[c]);
#endif
      bitboard_t &res = lookup[offset + pos];
      for (size_t direction = 0; direction < 4; ++direction) {
        coord_t p = c;
        for (;;) {
          res |= coordToBitboard(p);
          const subcoord_t nx = coordX(p) + static_cast<subcoord_t>(dx[direction]);
          const subcoord_t ny = coordY(p) + static_cast<subcoord_t>(dy[direction]);
          if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8 || (coordToBitboard(p) & occupied)) {
            break;
          }
          p = makeCoord(nx, ny);
        }
      }
      res &= ~coordToBitboard(c);
    }
  }
}

void initMagic() {
  initMagic<MagicType::Rook>();
  initMagic<MagicType::Bishop>();
}

}  // namespace SoFCore::Private
