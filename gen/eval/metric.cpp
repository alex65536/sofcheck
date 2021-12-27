// This file is part of SoFCheck
//
// Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
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

#include <algorithm>
#include <vector>

#include "common.h"
#include "core/types.h"

using namespace SoFCore;

std::vector<std::vector<bitboard_t>> generateKingMetricRings() {
  std::vector<std::vector<bitboard_t>> result(8, std::vector<bitboard_t>(64));
  for (SoFCore::coord_t i = 0; i < 64; ++i) {
    for (SoFCore::coord_t j = 0; j < 64; ++j) {
      const int distance = std::max(std::abs(SoFCore::coordX(i) - SoFCore::coordX(j)),
                                    std::abs(SoFCore::coordY(i) - SoFCore::coordY(j)));
      result[distance][i] |= static_cast<bitboard_t>(1) << j;
    }
  }
  return result;
}

GeneratorInfo getGeneratorInfo() {
  return GeneratorInfo{"Generate rings with the given distance for king"};
}

int doGenerate(SourcePrinter &p) {
  auto kingRings = generateKingMetricRings();

  p.headerGuard("SOF_EVAL_PRIVATE_METRIC_INCLUDED");
  p.skip();
  p.include("core/types.h");
  p.skip();
  auto ns = p.inNamespace("SoFEval::Private");
  p.skip();
  p.lineStart() << "constexpr SoFCore::bitboard_t KING_METRIC_RINGS[8][64] = ";
  p.arrayBody(8, [&](const size_t i) {
    p.arrayBody(64, [&](const size_t j) { printBitboard(p.stream(), kingRings[i][j]); });
  });
  p.line() << ";";
  p.skip();

  return 0;
}
