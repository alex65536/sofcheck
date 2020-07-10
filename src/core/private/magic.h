#ifndef MAGIC_H_INCLUDED
#define MAGIC_H_INCLUDED

#include "core/types.h"

#include <immintrin.h> // Use _pext_u64() from BMI2 instruction set

namespace SoFCore {
namespace Private {

struct MagicEntry {
  const bitboard_t *arr;
  bitboard_t msk; // Mask for _pext_u64()
  bitboard_t postMsk; // Mask to distinguish between shared attacks
};
 
extern MagicEntry g_magicRook[64];
extern MagicEntry g_magicBishop[64];

void initMagic();

inline bitboard_t rookAttackBitboard(bitboard_t occupied, cell_t pos) {
  const MagicEntry &entry = g_magicRook[pos];
  return entry.arr[_pext_u64(occupied, entry.msk)] & entry.postMsk;
}

inline bitboard_t bishopAttackBitboard(bitboard_t occupied, cell_t pos) {
  const MagicEntry &entry = g_magicBishop[pos];
  return entry.arr[_pext_u64(occupied, entry.msk)] & entry.postMsk;  
}

}
}

#endif // MAGIC_H_INCLUDED
