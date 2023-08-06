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

#include <cstddef>
#include <cstdint>
#include <vector>

#include "common.h"
#include "core/bitboard.h"
#include "core/types.h"

using namespace SoFCore;

template <typename Mask>
std::vector<bitboard_t> genBishopBetween(const Mask &mask) {
  std::vector<bitboard_t> result(64);
  for (coord_t c = 0; c < 64; ++c) {
    const auto bb = BB_DIAG1[coordDiag1(c)] | BB_DIAG2[coordDiag2(c)];
    result[c] = bb & mask(c);
  }
  return result;
}

template <typename Mask>
std::vector<bitboard_t> genRookBetween(const Mask &mask) {
  std::vector<bitboard_t> result(64);
  for (coord_t c = 0; c < 64; ++c) {
    const auto bb = BB_ROW[coordX(c)] | BB_COL[coordY(c)];
    result[c] = bb & mask(c);
  }
  return result;
}

inline bitboard_t ltMask(const coord_t c) {
  return coordToBitboard(c) - static_cast<bitboard_t>(1);
}

inline bitboard_t gtMask(const coord_t c) { return ~(ltMask(c) | coordToBitboard(c)); }

GeneratorInfo getGeneratorInfo() {
  return GeneratorInfo{"Generate tables to find cells located between pieces on one line"};
}

int doGenerate(SourcePrinter &p) {
  p.headerGuard("SOF_CORE_PRIVATE_BETWEEN_CONSTS_INCLUDED");
  p.skip();
  p.include("core/types.h");
  p.skip();
  auto ns = p.inNamespace("SoFCore::Private");
  p.skip();
  p.bitboardArray("BISHOP_LT", genBishopBetween(ltMask));
  p.skip();
  p.bitboardArray("BISHOP_GT", genBishopBetween(gtMask));
  p.skip();
  p.bitboardArray("ROOK_LT", genRookBetween(ltMask));
  p.skip();
  p.bitboardArray("ROOK_GT", genRookBetween(gtMask));
  p.skip();

  return 0;
}
