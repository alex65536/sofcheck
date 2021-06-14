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

#include <random>
#include <vector>

#include "common.h"
#include "core/private/magic_util.h"
#include "util/bit.h"

using namespace SoFCore;
using SoFCore::Private::MagicType;

template <MagicType M>
bool isValidMagic(coord_t coord, bitboard_t magic) {
  const bitboard_t mask = Private::buildMagicMask<M>(coord);
  const auto shift = SoFUtil::popcount(mask);
  const size_t submaskCnt = static_cast<size_t>(1U) << shift;
  std::vector<bool> used(submaskCnt);
  for (size_t submask = 0; submask < submaskCnt; ++submask) {
    const bitboard_t occupied = SoFUtil::depositBits(submask, mask);
    const auto idx = static_cast<size_t>((occupied * magic) >> (64 - shift));
    if (used[idx]) {
      return false;
    }
    used[idx] = true;
  }
  return true;
}

template <typename RandGen>
bitboard_t genSparseNumber(RandGen &rnd) {
  bitboard_t res = 0;
  for (size_t i = 0; i < 64; ++i) {
    res <<= 1;
    if (rnd() % 8 == 0) {
      res |= 1;
    }
  }
  return res;
}

template <MagicType M>
std::vector<bitboard_t> generateMagics() {
  std::mt19937_64 rnd(42);  // NOLINT: it's better to get the same magics during different builds,
                            // so the seed is fixed here
  std::vector<bitboard_t> result(64);

  for (coord_t i = 0; i < 64; ++i) {
    bitboard_t magic = 0;
    do {
      magic = genSparseNumber(rnd);
    } while (!isValidMagic<M>(i, magic));
    result[i] = magic;
  }

  return result;
}

template <MagicType M>
std::vector<coord_t> generateShifts() {
  std::vector<coord_t> shifts(64);
  for (coord_t i = 0; i < 64; ++i) {
    shifts[i] = static_cast<coord_t>(64 - Private::getMagicMaskBitSize<M>(i));
  }
  return shifts;
}

int doGenerate(SourcePrinter &p) {
  p.headerGuard("SOF_CORE_PRIVATE_MAGIC_CONSTANTS_INCLUDED");
  p.skip();
  p.include("core/types.h");
  p.skip();
  auto ns = p.inNamespace("SoFCore::Private");
  p.skip();
  p.bitboardArray("ROOK_MAGICS", generateMagics<MagicType::Rook>());
  p.skip();
  p.bitboardArray("BISHOP_MAGICS", generateMagics<MagicType::Bishop>());
  p.skip();
  p.coordArray("ROOK_SHIFTS", generateShifts<MagicType::Rook>());
  p.skip();
  p.coordArray("BISHOP_SHIFTS", generateShifts<MagicType::Bishop>());
  p.skip();

  return 0;
}
