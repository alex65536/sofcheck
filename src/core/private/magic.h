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

#ifndef SOF_CORE_PRIVATE_MAGIC_INCLUDED
#define SOF_CORE_PRIVATE_MAGIC_INCLUDED

#include <cstddef>

#include "config.h"
#include "core/types.h"

#ifdef USE_BMI2
#include <immintrin.h>
#else
#include "core/private/magic_consts.h"
#endif

namespace SoFCore::Private {

struct MagicEntry {
  const bitboard_t *lookup;
  bitboard_t mask;
  bitboard_t postMask;
};

extern MagicEntry g_magicRook[64];
extern MagicEntry g_magicBishop[64];

void initMagic();

inline bitboard_t rookAttackBitboard(bitboard_t occupied, cell_t pos) {
  const MagicEntry &entry = g_magicRook[pos];
#ifdef USE_BMI2
  const size_t idx = _pext_u64(occupied, entry.mask);
#else
  const auto idx =
      static_cast<size_t>(((occupied & entry.mask) * ROOK_MAGICS[pos]) >> ROOK_SHIFTS[pos]);
#endif
  return entry.lookup[idx] & entry.postMask;
}

inline bitboard_t bishopAttackBitboard(bitboard_t occupied, cell_t pos) {
  const MagicEntry &entry = g_magicBishop[pos];
#ifdef USE_BMI2
  const size_t idx = _pext_u64(occupied, entry.mask);
#else
  const auto idx =
      static_cast<size_t>(((occupied & entry.mask) * BISHOP_MAGICS[pos]) >> BISHOP_SHIFTS[pos]);
#endif
  return entry.lookup[idx] & entry.postMask;
}

}  // namespace SoFCore::Private

#endif  // SOF_CORE_PRIVATE_MAGIC_INCLUDED
