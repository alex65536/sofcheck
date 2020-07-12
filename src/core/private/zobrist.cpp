#include "core/private/zobrist.h"

#include <chrono>
#include <random>

#include "core/private/rows.h"
#include "core/types.h"

namespace SoFCore::Private {

board_hash_t g_zobristPieces[16][64];
board_hash_t g_zobristMoveSide;
board_hash_t g_zobristCastling[16];
board_hash_t g_zobristEnpassant[64];
board_hash_t g_zobristPieceCastlingKingside[2];
board_hash_t g_zobristPieceCastlingQueenside[2];

void initZobrist() {
  std::mt19937_64 rnd(std::chrono::steady_clock::now().time_since_epoch().count());
  for (size_t j = 0; j < 64; ++j) {
    g_zobristPieces[0][j] = 0;
  }
  for (size_t i = 1; i < 16; ++i) {
    for (size_t j = 0; j < 64; ++j) {
      g_zobristPieces[i][j] = rnd();
    }
  }
  g_zobristMoveSide = rnd();
  for (board_hash_t &hash : g_zobristCastling) {
    hash = rnd();
  }
  for (board_hash_t &hash : g_zobristEnpassant) {
    hash = rnd();
  }
  for (Color c : {Color::White, Color::Black}) {
    const auto idx = static_cast<size_t>(c);
    const coord_t firstRowStart = castlingRow(c) << 3;
    const cell_t king = makeCell(c, Piece::King);
    const cell_t rook = makeCell(c, Piece::Rook);
    g_zobristPieceCastlingKingside[idx] = Private::g_zobristPieces[king][firstRowStart + 4] ^
                                          Private::g_zobristPieces[rook][firstRowStart + 5] ^
                                          Private::g_zobristPieces[king][firstRowStart + 6] ^
                                          Private::g_zobristPieces[rook][firstRowStart + 7];
    g_zobristPieceCastlingQueenside[idx] =
        g_zobristPieces[rook][firstRowStart + 0] ^ g_zobristPieces[king][firstRowStart + 2] ^
        g_zobristPieces[rook][firstRowStart + 3] ^ g_zobristPieces[king][firstRowStart + 4];
  }
}

}  // namespace SoFCore::Private
