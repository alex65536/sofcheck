#ifndef MAGIC_H_INCLUDED
#define MAGIC_H_INCLUDED

#include "config.h"
#include "core/types.h"

#ifdef USE_BMI2
#include <immintrin.h>
#endif

namespace SoFCore {
namespace Private {

struct MagicEntry {
  const bitboard_t *lookup;
  bitboard_t mask;
  bitboard_t postMask;
};

extern MagicEntry g_magicRook[64];
extern MagicEntry g_magicBishop[64];

void initMagic();

inline bitboard_t rookAttackBitboard(bitboard_t occupied, cell_t pos) {
  const MagicEntry &entry = g_magicRook[pos];
  const size_t idx = _pext_u64(occupied, entry.mask);
  return entry.lookup[idx] & entry.postMask;
}

inline bitboard_t bishopAttackBitboard(bitboard_t occupied, cell_t pos) {
  const MagicEntry &entry = g_magicBishop[pos];
  const size_t idx = _pext_u64(occupied, entry.mask);
  return entry.lookup[idx] & entry.postMask;
}

}  // namespace Private
}  // namespace SoFCore

#endif  // MAGIC_H_INCLUDED
