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
