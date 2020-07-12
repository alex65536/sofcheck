#ifndef BIT_H_INCLUDED
#define BIT_H_INCLUDED

#include <cstdint>

#include "config.h"

#ifdef USE_BMI2
#include <immintrin.h>
#endif

namespace SoFUtil {

// Returns the number of ones in x
inline constexpr uint8_t popcount(uint64_t x) { return __builtin_popcountll(x); }

// Clears the lowest bit set to one
inline constexpr uint64_t clearLowest(uint64_t x) { return x & (x - 1); }

// Returns the lowest bit number
// If x == 0, the behavior is undefined!
inline constexpr uint8_t getLowest(uint64_t x) { return __builtin_ctzll(x); }

// Combines getLowest and clearLowest for convenience
// If x == 0, the behavior is undefined!
inline constexpr uint8_t extractLowest(uint64_t &x) {
  uint8_t res = getLowest(x);
  x = clearLowest(x);
  return res;
}

#ifdef USE_BMI2

// The function does the same as _pdep_u64 Intel intrinsic (or PDEP instruction)
inline uint64_t depositBits(uint64_t x, uint64_t msk) { return _pdep_u64(x, msk); }

#else

// The function does the same as _pdep_u64 Intel intrinsic (or PDEP instruction)
// This is a naive implementation for CPUs that do not support BMI2
inline uint64_t depositBits(uint64_t x, uint64_t msk) {
  uint64_t res = 0;
  while (msk) {
    const uint64_t bit = msk & -msk;
    if (x & 1) {
      res |= bit;
    }
    msk ^= bit;
    x >>= 1;
  }
  return res;
}

#endif

}  // namespace SoFUtil

#endif  // BIT_H_INCLUDED
