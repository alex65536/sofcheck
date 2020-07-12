#ifndef MAGIC_UTIL_H_INCLUDED
#define MAGIC_UTIL_H_INCLUDED

#include "core/private/bit_consts.h"
#include "core/types.h"

namespace SoFCore {
namespace Private {

inline constexpr bitboard_t buildMagicRookMask(const coord_t c) {
  const subcoord_t x = coordX(c);
  const subcoord_t y = coordY(c);
  return ((BB_ROW[x] & ~BB_ROW_FRAME) | (BB_COL[y] & ~BB_COL_FRAME)) & ~coordToBitboard(c);
}

inline constexpr bitboard_t buildMagicRookPostMask(const coord_t c) {
  const subcoord_t x = coordX(c);
  const subcoord_t y = coordY(c);
  return BB_ROW[x] ^ BB_COL[y];
}

inline constexpr bitboard_t buildMagicBishopMask(const coord_t c) {
  const subcoord_t d1 = coordX(c) + coordY(c);
  const subcoord_t d2 = 7 - coordX(c) + coordY(c);
  return (BB_DIAG1[d1] ^ BB_DIAG2[d2]) & ~BB_DIAG_FRAME;  
}

inline constexpr bitboard_t buildMagicBishopPostMask(const coord_t c) {
  const subcoord_t d1 = coordX(c) + coordY(c);
  const subcoord_t d2 = 7 - coordX(c) + coordY(c);
  return BB_DIAG1[d1] ^ BB_DIAG2[d2];  
}

}  // namespace Private
}  // namespace SoFCore

#endif  // MAGIC_UTIL_H_INCLUDED
