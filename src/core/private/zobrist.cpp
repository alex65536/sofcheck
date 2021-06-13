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

#include "core/private/zobrist.h"

#include <initializer_list>

#include "core/private/geometry.h"
#include "core/types.h"
#include "util/random.h"

namespace SoFCore::Private {

board_hash_t g_zobristPieces[16][64];
board_hash_t g_zobristMoveSide;
board_hash_t g_zobristCastling[16];
board_hash_t g_zobristEnpassant[64];
board_hash_t g_zobristPieceCastlingKingside[2];
board_hash_t g_zobristPieceCastlingQueenside[2];

void initZobrist() {
  using SoFUtil::random;

  for (size_t j = 0; j < 64; ++j) {
    g_zobristPieces[0][j] = 0;
  }
  for (size_t i = 1; i < 16; ++i) {
    for (size_t j = 0; j < 64; ++j) {
      g_zobristPieces[i][j] = random();
    }
  }
  g_zobristMoveSide = random();
  for (board_hash_t &hash : g_zobristCastling) {
    hash = random();
  }
  for (board_hash_t &hash : g_zobristEnpassant) {
    hash = random();
  }
  for (Color c : {Color::White, Color::Black}) {
    const auto idx = static_cast<size_t>(c);
    const coord_t offset = Private::castlingOffset(c);
    const cell_t king = makeCell(c, Piece::King);
    const cell_t rook = makeCell(c, Piece::Rook);
    g_zobristPieceCastlingKingside[idx] =
        Private::g_zobristPieces[king][offset + 4] ^ Private::g_zobristPieces[rook][offset + 5] ^
        Private::g_zobristPieces[king][offset + 6] ^ Private::g_zobristPieces[rook][offset + 7];
    g_zobristPieceCastlingQueenside[idx] =
        g_zobristPieces[rook][offset + 0] ^ g_zobristPieces[king][offset + 2] ^
        g_zobristPieces[rook][offset + 3] ^ g_zobristPieces[king][offset + 4];
  }
}

}  // namespace SoFCore::Private
