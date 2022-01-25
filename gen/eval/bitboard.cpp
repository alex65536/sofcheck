// This file is part of SoFCheck
//
// Copyright (c) 2021-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#include "core/bitboard.h"  // This header must not be higher than the others, but `clang-format`
                            // puts it here by mistake

#include <algorithm>
#include <vector>

#include "common.h"
#include "core/types.h"
#include "util/math.h"

using SoFCore::BB_COL;
using SoFCore::bitboard_t;
using SoFCore::Color;
using SoFCore::coord_t;
using SoFCore::subcoord_t;

std::vector<std::vector<bitboard_t>> generateKingMetricRings() {
  std::vector<std::vector<bitboard_t>> result(8, std::vector<bitboard_t>(64));
  for (coord_t i = 0; i < 64; ++i) {
    for (coord_t j = 0; j < 64; ++j) {
      const int distance = std::max(SoFUtil::absDiff(SoFCore::coordX(i), SoFCore::coordX(j)),
                                    SoFUtil::absDiff(SoFCore::coordY(i), SoFCore::coordY(j)));
      result[distance][i] |= SoFCore::coordToBitboard(j);
    }
  }
  return result;
}

std::vector<bitboard_t> generateDoublePawns() {
  std::vector<bitboard_t> result(64);
  for (coord_t i = 0; i < 64; ++i) {
    result[i] = BB_COL[SoFCore::coordY(i)] ^ SoFCore::coordToBitboard(i);
  }
  return result;
}

std::vector<bitboard_t> generateIsolatedPawns() {
  std::vector<bitboard_t> result(64);
  for (coord_t i = 0; i < 64; ++i) {
    const subcoord_t y = SoFCore::coordY(i);
    if (y != 0) {
      result[i] |= BB_COL[y - 1];
    }
    if (y != 7) {
      result[i] |= BB_COL[y + 1];
    }
  }
  return result;
}

enum class PassedPawnKind { Passed, Open };

std::vector<bitboard_t> generatePassedOrOpenPawns(const Color c, const PassedPawnKind kind) {
  std::vector<bitboard_t> result(64);
  for (coord_t i = 0; i < 64; ++i) {
    for (coord_t j = 0; j < 64; ++j) {
      const subcoord_t yi = SoFCore::coordY(i);
      const subcoord_t yj = SoFCore::coordY(j);
      const subcoord_t xi = SoFCore::coordX(i);
      const subcoord_t xj = SoFCore::coordX(j);
      if ((kind == PassedPawnKind::Passed && yi != yj && yi + 1 != yj && yi != yj + 1) ||
          (kind == PassedPawnKind::Open && yi != yj)) {
        continue;
      }
      if ((c == Color::White && xi > xj) || (c == Color::Black && xi < xj)) {
        result[i] |= SoFCore::coordToBitboard(j);
      }
    }
  }
  return result;
}

std::vector<bitboard_t> generateAttackFrontspans(const Color c) {
  std::vector<bitboard_t> result(64);
  for (coord_t i = 0; i < 64; ++i) {
    for (coord_t j = 0; j < 64; ++j) {
      const subcoord_t yi = SoFCore::coordY(i);
      const subcoord_t yj = SoFCore::coordY(j);
      const subcoord_t xi = SoFCore::coordX(i);
      const subcoord_t xj = SoFCore::coordX(j);
      if (yi + 1 != yj && yi != yj + 1) {
        continue;
      }
      if ((c == Color::White && xi > xj) || (c == Color::Black && xi < xj)) {
        result[i] |= SoFCore::coordToBitboard(j);
      }
    }
  }
  return result;
}

GeneratorInfo getGeneratorInfo() {
  return GeneratorInfo{"Generate various bitboard-related constants needed for evaluation"};
}

int doGenerate(SourcePrinter &p) {
  const auto kingRings = generateKingMetricRings();

  p.headerGuard("SOF_EVAL_PRIVATE_BITBOARD_INCLUDED");
  p.skip();
  p.include("core/types.h");
  p.skip();
  auto ns = p.inNamespace("SoFEval::Private");
  p.skip();
  p.lineStart() << "constexpr SoFCore::bitboard_t BB_KING_METRIC_RING[8][64] = ";
  p.arrayBody(8, [&](const size_t i) {
    p.arrayBody(64, [&](const size_t j) { printBitboard(p.stream(), kingRings[i][j]); });
  });
  p.line() << ";";
  p.skip();
  p.bitboardArray("BB_DOUBLE_PAWN", generateDoublePawns());
  p.skip();
  p.bitboardArray("BB_ISOLATED_PAWN", generateIsolatedPawns());
  p.skip();
  p.bitboardArray("BB_PASSED_PAWN_WHITE",
                  generatePassedOrOpenPawns(Color::White, PassedPawnKind::Passed));
  p.skip();
  p.bitboardArray("BB_PASSED_PAWN_BLACK",
                  generatePassedOrOpenPawns(Color::Black, PassedPawnKind::Passed));
  p.skip();
  p.bitboardArray("BB_OPEN_PAWN_WHITE",
                  generatePassedOrOpenPawns(Color::White, PassedPawnKind::Open));
  p.skip();
  p.bitboardArray("BB_OPEN_PAWN_BLACK",
                  generatePassedOrOpenPawns(Color::Black, PassedPawnKind::Open));
  p.skip();
  p.bitboardArray("BB_ATTACK_FRONTSPANS_WHITE", generateAttackFrontspans(Color::White));
  p.skip();
  p.bitboardArray("BB_ATTACK_FRONTSPANS_BLACK", generateAttackFrontspans(Color::Black));
  p.skip();
  p.line() << "constexpr SoFCore::bitboard_t BB_WHITE_SHIELDED_KING = 0xc300000000000000;";
  p.line() << "constexpr SoFCore::bitboard_t BB_BLACK_SHIELDED_KING = 0x00000000000000c3;";
  p.skip();

  return 0;
}
