#include "magic.h"

#include <algorithm>
#include <iostream>

#include "util/bit.h"

namespace SoFCore {
namespace Private {

MagicEntry g_magicRook[64];
MagicEntry g_magicBishop[64];

constexpr bitboard_t BB_DIAG1[15] = {
    0x0000000000000001, 0x0000000000000102, 0x0000000000010204, 0x0000000001020408,
    0x0000000102040810, 0x0000010204081020, 0x0001020408102040, 0x0102040810204080,
    0x0204081020408000, 0x0408102040800000, 0x0810204080000000, 0x1020408000000000,
    0x2040800000000000, 0x4080000000000000, 0x8000000000000000,
};

constexpr bitboard_t BB_DIAG2[15] = {
    0x0100000000000000, 0x0201000000000000, 0x0402010000000000, 0x0804020100000000,
    0x1008040201000000, 0x2010080402010000, 0x4020100804020100, 0x8040201008040201,
    0x0080402010080402, 0x0000804020100804, 0x0000008040201008, 0x0000000080402010,
    0x0000000000804020, 0x0000000000008040, 0x0000000000000080,
};

constexpr bitboard_t BB_ROW[8] = {
    0x00000000000000ff, 0x000000000000ff00, 0x0000000000ff0000, 0x00000000ff000000,
    0x000000ff00000000, 0x0000ff0000000000, 0x00ff000000000000, 0xff00000000000000,
};

constexpr bitboard_t BB_COL[8] = {
    0x0101010101010101, 0x0202020202020202, 0x0404040404040404, 0x0808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
};

constexpr bitboard_t BB_ROW_FRAME = 0x8181818181818181;
constexpr bitboard_t BB_COL_FRAME = 0xff000000000000ff;
constexpr bitboard_t BB_DIAG_FRAME = 0xff818181818181ff;

bitboard_t g_rookAttacks[65536];
bitboard_t g_bishopAttacks[1792];

static void initRookMagic() {
  size_t rookPos = 0;

  // First, generate attack masks
  for (coord_t c = 0; c < 64; ++c) {
    const subcoord_t x = coordX(c);
    const subcoord_t y = coordY(c);
    g_magicRook[c].postMsk = BB_ROW[x] ^ BB_COL[y];
    g_magicRook[c].msk =
        ((BB_ROW[x] & ~BB_ROW_FRAME) | (BB_COL[y] & ~BB_COL_FRAME)) & ~coordToBitboard(c);
  }

  // Determine pointers for shared rook arrays (one entry for both c1 and c2)
  // For details, see https://www.chessprogramming.org/Magic_Bitboards#Sharing_Attacks
  size_t bases[64];
  for (coord_t c1 = 0; c1 < 64; ++c1) {
    const coord_t c2 = c1 ^ 9;
    if (c1 > c2) {
      continue;
    }
    const int maxLen =
        std::max(SoFUtil::popcount(g_magicRook[c1].msk), SoFUtil::popcount(g_magicRook[c2].msk));
    const size_t add = 1L << maxLen;
    g_magicRook[c1].arr = g_rookAttacks + rookPos;
    g_magicRook[c2].arr = g_rookAttacks + rookPos;
    bases[c1] = rookPos;
    bases[c2] = rookPos;
    rookPos += add;
  }

  // Fill g_rookAttacks
  for (coord_t c = 0; c < 64; ++c) {
    const size_t len = 1L << SoFUtil::popcount(g_magicRook[c].msk);
    const size_t base = bases[c];
    for (size_t pos = 0; pos < len; ++pos) {
      const bitboard_t occupied = _pdep_u64(pos, g_magicRook[c].msk);
      bitboard_t &res = g_rookAttacks[base + pos];
      const int8_t dx[4] = {-1, 1, 0, 0};
      const int8_t dy[4] = {0, 0, -1, 1};
      for (int direction = 0; direction < 4; ++direction) {
        coord_t p = c;
        for (;;) {
          subcoord_t nx = coordX(p) + static_cast<subcoord_t>(dx[direction]);
          subcoord_t ny = coordY(p) + static_cast<subcoord_t>(dy[direction]);
          if (nx >= 8 || ny >= 8 || (coordToBitboard(p) & occupied)) {
            res |= coordToBitboard(p);
            break;
          }
          p = makeCoord(nx, ny);
        }
      }
    }
  }
}

static void initBishopMagic() {
  size_t bishopPos = 0;

  // First, generate attack masks
  for (coord_t c = 0; c < 64; ++c) {
    const subcoord_t d1 = coordX(c) + coordY(c);
    const subcoord_t d2 = coordX(c) + 7 - coordY(c);
    g_magicBishop[c].postMsk = BB_DIAG1[d1] ^ BB_DIAG2[d2];
    g_magicBishop[c].msk = (BB_DIAG1[d1] ^ BB_DIAG2[d2]) & ~BB_DIAG_FRAME;
  }

  // Determine pointers for shared bishop arrays (one entry for four cells)
  // For details, see https://www.chessprogramming.org/Magic_Bitboards#Sharing_Attackss
  const coord_t starts[16] = {0, 1, 32, 33, 2, 10, 18, 26, 34, 42, 50, 58, 6, 7, 38, 39};
  const coord_t offsets[16] = {8, 8, 8, 8, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 8, 8};
  size_t bases[64];
  for (size_t idx = 0; idx < 16; ++idx) {
    const coord_t c = starts[idx];
    const coord_t offs = offsets[idx];
    const int maxLen = std::max(std::max(SoFUtil::popcount(g_magicBishop[c + 0 * offs].msk),
                                         SoFUtil::popcount(g_magicBishop[c + 1 * offs].msk)),
                                std::max(SoFUtil::popcount(g_magicBishop[c + 2 * offs].msk),
                                         SoFUtil::popcount(g_magicBishop[c + 3 * offs].msk)));
    const size_t add = 1L << maxLen;
    for (coord_t i = 0; i < 4; ++i) {
      g_magicBishop[c + i * offs].arr = g_bishopAttacks + bishopPos;
      bases[c + i * offs] = bishopPos;
    }
    bishopPos += add;
  }

  // Fill g_bishopAttacks
  for (coord_t c = 0; c < 64; ++c) {
    const size_t len = 1L << SoFUtil::popcount(g_magicBishop[c].msk);
    const size_t base = bases[c];
    const int8_t dx[4] = {-1, 1, -1, 1};
    const int8_t dy[4] = {-1, -1, 1, 1};
    for (size_t pos = 0; pos < len; ++pos) {
      const bitboard_t occupied = _pdep_u64(pos, g_magicBishop[c].msk);
      bitboard_t &res = g_bishopAttacks[base + pos];
      for (int direction = 0; direction < 4; ++direction) {
        coord_t p = c;
        for (;;) {
          subcoord_t nx = coordX(p) + static_cast<subcoord_t>(dx[direction]);
          subcoord_t ny = coordY(p) + static_cast<subcoord_t>(dy[direction]);
          if (nx >= 8 || ny >= 8 || (coordToBitboard(p) & occupied)) {
            res |= coordToBitboard(p);
            break;
          }
          p = makeCoord(nx, ny);
        }
      }
    }
  }
}

void initMagic() {
  initRookMagic();
  initBishopMagic();
}

}  // namespace Private
}  // namespace SoFCore
