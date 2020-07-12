#include "magic.h"

#include <algorithm>
#include <iostream>

#include "core/private/magic_util.h"
#include "util/bit.h"

namespace SoFCore {
namespace Private {

MagicEntry g_magicRook[64];
MagicEntry g_magicBishop[64];

static bitboard_t g_rookLookup[65536];
static bitboard_t g_bishopLookup[1792];

enum class MagicType { Rook, Bishop };

template <MagicType M>
inline static constexpr const char *getMagicTypeName() {
  return (M == MagicType::Rook) ? "Rook" : "Bishop";
}

template <MagicType M>
inline static constexpr size_t buildMagicMask(coord_t c) {
  return (M == MagicType::Rook) ? buildMagicRookMask(c) : buildMagicBishopMask(c);
}

template <MagicType M>
inline static constexpr size_t getMagicMaskBitSize(coord_t c) {
  return SoFUtil::popcount(buildMagicMask<M>(c));
}

// Determine pointers for shared rook and bishop arrays
// For rooks, we share two cells (one entry for both c1 and c2)
// For bishops the number of shared cells is equal to four
// To find more details, see https://www.chessprogramming.org/Magic_Bitboards#Sharing_Attacks
template <MagicType M>
inline static void initOffsets(size_t bases[]) {
  size_t count = 0;
  if constexpr (M == MagicType::Rook) {
    for (coord_t c1 = 0; c1 < 64; ++c1) {
      const coord_t c2 = c1 ^ 9;
      if (c1 > c2) {
        continue;
      }
      const int maxLen = std::max(getMagicMaskBitSize<M>(c1), getMagicMaskBitSize<M>(c2));
      const size_t add = 1L << maxLen;
      bases[c1] = count;
      bases[c2] = count;
      count += add;
    }
  } else {
    const coord_t starts[16] = {0, 1, 32, 33, 2, 10, 18, 26, 34, 42, 50, 58, 6, 7, 38, 39};
    const coord_t offsets[16] = {8, 8, 8, 8, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8, 8, 8};
    for (size_t idx = 0; idx < 16; ++idx) {
      const coord_t c = starts[idx];
      const coord_t offs = offsets[idx];
      const int maxLen = std::max(
          std::max(getMagicMaskBitSize<M>(c + 0 * offs), getMagicMaskBitSize<M>(c + 1 * offs)),
          std::max(getMagicMaskBitSize<M>(c + 2 * offs), getMagicMaskBitSize<M>(c + 3 * offs)));
      const size_t add = 1L << maxLen;
      for (coord_t i = 0; i < 4; ++i) {
        bases[c + i * offs] = count;
      }
      count += add;
    }
  }
}

template <MagicType M>
static void initMagic() {
  MagicEntry *magicEntries = (M == MagicType::Rook) ? g_magicRook : g_magicBishop;
  bitboard_t *lookup = (M == MagicType::Rook ? g_rookLookup : g_bishopLookup);

  // First, generate attack masks
  for (coord_t c = 0; c < 64; ++c) {
    if constexpr (M == MagicType::Rook) {
      magicEntries[c].mask = buildMagicRookMask(c);
      magicEntries[c].postMask = buildMagicRookPostMask(c);
    } else {
      magicEntries[c].mask = buildMagicBishopMask(c);
      magicEntries[c].postMask = buildMagicBishopPostMask(c);
    }
  }

  // Fill table offsets
  size_t offsets[64];
  initOffsets<M>(offsets);
  for (coord_t c = 0; c < 64; ++c) {
    magicEntries[c].lookup = lookup + offsets[c];
  }

  // Fill lookup table
  for (coord_t c = 0; c < 64; ++c) {
    const bitboard_t mask = magicEntries[c].mask;
    const size_t len = 1L << SoFUtil::popcount(mask);
    const size_t offset = offsets[c];
    const int8_t dxBishop[4] = {-1, 1, -1, 1};
    const int8_t dyBishop[4] = {-1, -1, 1, 1};
    const int8_t dxRook[4] = {-1, 1, 0, 0};
    const int8_t dyRook[4] = {0, 0, -1, 1};
    const int8_t *dx = (M == MagicType::Rook) ? dxRook : dxBishop;
    const int8_t *dy = (M == MagicType::Rook) ? dyRook : dyBishop;
    for (size_t idx = 0; idx < len; ++idx) {
      const bitboard_t occupied = SoFUtil::depositBits(idx, mask);
#ifdef USE_BMI2
      const size_t pos = idx;
#else
      const bitboard_t *magics = (M == MagicType::Rook) ? ROOK_MAGICS : BISHOP_MAGICS;
      const coord_t *shifts = (M == MagicType::Rook) ? ROOK_SHIFTS : BISHOP_SHIFTS;
      const size_t pos = (occupied * magics[c]) >> shifts[c];
#endif
      bitboard_t &res = lookup[offset + pos];
      for (int direction = 0; direction < 4; ++direction) {
        coord_t p = c;
        for (;;) {
          res |= coordToBitboard(p);
          const subcoord_t nx = coordX(p) + static_cast<subcoord_t>(dx[direction]);
          const subcoord_t ny = coordY(p) + static_cast<subcoord_t>(dy[direction]);
          if (nx >= 8 || ny >= 8 || (coordToBitboard(p) & occupied)) {
            break;
          }
          p = makeCoord(nx, ny);
        }
      }
      res &= ~coordToBitboard(c);
    }
  }
}

void initMagic() {
  initMagic<MagicType::Rook>();
  initMagic<MagicType::Bishop>();
}

}  // namespace Private
}  // namespace SoFCore
