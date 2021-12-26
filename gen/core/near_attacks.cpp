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

#include <cstddef>
#include <cstdint>
#include <vector>

#include "common.h"
#include "core/types.h"

using namespace SoFCore;

std::vector<bitboard_t> generateDirected(const int8_t offX[], const int8_t offY[],
                                         const size_t size) {
  std::vector<bitboard_t> results(64);

  for (coord_t c = 0; c < 64; ++c) {
    bitboard_t bb = 0;
    const subcoord_t x = coordX(c);
    const subcoord_t y = coordY(c);
    for (size_t direction = 0; direction < size; ++direction) {
      const subcoord_t nx = x + static_cast<subcoord_t>(offX[direction]);
      const subcoord_t ny = y + static_cast<subcoord_t>(offY[direction]);
      if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
        bb |= coordToBitboard(makeCoord(nx, ny));
      }
    }
    results[c] = bb;
  }

  return results;
}

std::vector<bitboard_t> generateKnightAttacks() {
  const int8_t offX[] = {-2, -2, -1, -1, 2, 2, 1, 1};
  const int8_t offY[] = {-1, 1, -2, 2, -1, 1, -2, 2};
  return generateDirected(offX, offY, 8);
}

std::vector<bitboard_t> generateKingAttacks() {
  const int8_t offX[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  const int8_t offY[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  return generateDirected(offX, offY, 8);
}

std::vector<bitboard_t> generateWhitePawnAttacks() {
  const int8_t offX[] = {-1, -1};
  const int8_t offY[] = {-1, 1};
  return generateDirected(offX, offY, 2);
}

std::vector<bitboard_t> generateBlackPawnAttacks() {
  const int8_t offX[] = {1, 1};
  const int8_t offY[] = {-1, 1};
  return generateDirected(offX, offY, 2);
}

GeneratorInfo getGeneratorInfo() {
  return GeneratorInfo{"Generate tables for near attacks"};
}

int doGenerate(SourcePrinter &p) {
  auto knightAttacks = generateKnightAttacks();
  auto kingAttacks = generateKingAttacks();
  auto whitePawnAttacks = generateWhitePawnAttacks();
  auto blackPawnAttacks = generateBlackPawnAttacks();

  p.headerGuard("SOF_CORE_PRIVATE_NEAR_ATTACKS_INCLUDED");
  p.skip();
  p.include("core/types.h");
  p.skip();
  auto ns = p.inNamespace("SoFCore::Private");
  p.skip();
  p.bitboardArray("KING_ATTACKS", kingAttacks);
  p.skip();
  p.bitboardArray("KNIGHT_ATTACKS", knightAttacks);
  p.skip();
  p.bitboardArray("WHITE_PAWN_ATTACKS", whitePawnAttacks);
  p.skip();
  p.bitboardArray("BLACK_PAWN_ATTACKS", blackPawnAttacks);
  p.skip();

  return 0;
}
