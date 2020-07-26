#ifndef SOF_CORE_PRIVATE_MAGIC_UTIL_INCLUDED
#define SOF_CORE_PRIVATE_MAGIC_UTIL_INCLUDED

#include "core/private/bit_consts.h"
#include "core/types.h"
#include "util/bit.h"

namespace SoFCore::Private {

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

enum class MagicType { Rook, Bishop };

template <MagicType M>
inline constexpr bitboard_t buildMagicMask(const coord_t c) {
  return (M == MagicType::Rook) ? buildMagicRookMask(c) : buildMagicBishopMask(c);
}

template <MagicType M>
inline constexpr bitboard_t buildMagicPostMask(const coord_t c) {
  return (M == MagicType::Rook) ? buildMagicRookPostMask(c) : buildMagicBishopPostMask(c);
}

template <MagicType M>
inline constexpr size_t getMagicMaskBitSize(const coord_t c) {
  return SoFUtil::popcount(buildMagicMask<M>(c));
}

}  // namespace SoFCore::Private

#endif  // SOF_CORE_PRIVATE_MAGIC_UTIL_INCLUDED
