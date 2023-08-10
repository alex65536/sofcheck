// This file is part of SoFCheck
//
// Copyright (c) 2020-2023 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_UTIL_BIT_INCLUDED
#define SOF_UTIL_BIT_INCLUDED

// Switch between various implementations of bit manipulation routines
#if defined(_WIN64) && defined(_MSC_VER)
#define SOF_BIT_MSVC64
#elif defined(__GNUC__)
#define SOF_BIT_GCC
#else
#define SOF_BIT_UNKNOWN
#endif

#include <cstdint>

#include "config.h"

#ifdef SOF_BIT_MSVC64
#include <intrin.h>

#include <cstdlib>
#endif

#ifdef USE_BMI2
#include <immintrin.h>
#endif

namespace SoFUtil {

// Returns the number of ones in `x`
#if defined(SOF_BIT_GCC) && defined(USE_POPCNT)
inline size_t popcount(uint64_t x) { return __builtin_popcountll(x); }
#elif defined(SOF_BIT_MSVC64) && defined(USE_POPCNT)
inline size_t popcount(uint64_t x) { return __popcnt64(x); }
#else
// See https://www.chessprogramming.org/Population_Count#SWAR-Popcount for more details. This
// implementation is not ultra-fast, but can be used as fallback
inline size_t popcount(uint64_t x) {
  x -= (x >> 1) & 0x5555555555555555ULL;
  x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
  x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
  return static_cast<size_t>((x * 0x0101010101010101ULL) >> 56);
}
#endif

// Returns true if `popcount(x) <= 1`
inline constexpr uint64_t hasZeroOrOneBit(uint64_t x) { return (x & (x - 1)) == 0; }

// Clears the lowest bit set to one in `x`
inline constexpr uint64_t clearLowest(uint64_t x) { return x & (x - 1); }

// Returns the position of the lowest bit set to one in `x`. If `x == 0`, the behavior is undefined
#if defined(SOF_BIT_GCC)
inline size_t getLowest(uint64_t x) { return __builtin_ctzll(x); }
#elif defined(SOF_BIT_MSVC64)
inline size_t getLowest(uint64_t x) {
  unsigned long result;
  _BitScanForward64(&result, x);
  return static_cast<size_t>(result);
}
#else
// See https://www.chessprogramming.org/BitScan#De_Bruijn_Multiplication for details. This
// implementation is used as fallback
inline size_t getLowest(uint64_t x) {
  constexpr uint8_t INDEX[64] = {
      0,  47, 1,  56, 48, 27, 2,  60,  //
      57, 49, 41, 37, 28, 16, 3,  61,  //
      54, 58, 35, 52, 50, 42, 21, 44,  //
      38, 32, 29, 23, 17, 11, 4,  62,  //
      46, 55, 26, 59, 40, 36, 15, 53,  //
      34, 51, 20, 43, 31, 22, 10, 45,  //
      25, 39, 14, 33, 19, 30, 9,  24,  //
      13, 18, 8,  12, 7,  6,  5,  63   //
  };

  const uint8_t result = INDEX[((x ^ (x - 1)) * 0x03f79d71b4cb0a89ULL) >> 58];
  return result;
}
#endif

// Combines `getLowest` and `clearLowest` for convenience. It clears the lowest bit set to one in
// `x` and returns the position of the cleared bit. If `x == 0`, the behavior is undefined
inline size_t extractLowest(uint64_t &x) {
  const size_t res = getLowest(x);
  x = clearLowest(x);
  return res;
}

// Reverses the byte order in `x`
#if defined(SOF_BIT_GCC)
inline uint64_t swapBytes(uint64_t x) { return __builtin_bswap64(x); }
#elif defined(SOF_BIT_MSVC64)
inline uint64_t swapBytes(uint64_t x) { return _byteswap_uint64(x); }
#else
// See https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating#Vertical for details. This
// implementation is used as fallback
inline uint64_t swapBytes(uint64_t x) {
  x = ((x >> 8) & 0x00ff00ff00ff00ffULL) | ((x & 0x00ff00ff00ff00ffULL) << 8);
  x = ((x >> 16) & 0x0000ffff0000ffffULL) | ((x & 0x0000ffff0000ffffULL) << 16);
  x = (x >> 32) | (x << 32);
  return x;
}
#endif

// Calculates bitwise OR over all the bytes in 8-byte number `x`
inline constexpr uint8_t byteGather(uint64_t x) {
  x |= x >> 32;
  x |= x >> 16;
  x |= x >> 8;
  return static_cast<uint8_t>(x);
}

// Creates a 8-bit number, with all of its bytes equal to `x`
inline constexpr uint64_t byteScatter(uint8_t x) {
  uint64_t r = x;
  r |= r << 8;
  r |= r << 16;
  r |= r << 32;
  return r;
}

// Performs a left rotation of `x` by `shift`. If `shift >= 64`, the behavior is undefined
inline constexpr uint64_t rotateLeft(const uint64_t x, const size_t shift) {
  return (shift == 0) ? x : ((x << shift) | (x >> (64 - shift)));
}

// Performs a right rotation of `x` by `shift`. If `shift >= 64`, the behavior is undefined
inline constexpr uint64_t rotateRight(const uint64_t x, const size_t shift) {
  return (shift == 0) ? x : ((x >> shift) | (x << (64 - shift)));
}

// The function does the same as `_pdep_u64` Intel intrinsic (or `PDEP` Intel instruction)
//
// For more details, you can look at https://www.felixcloutier.com/x86/pdep
#ifdef USE_BMI2
// If BMI2 is supported, just use PDEP instruction
inline uint64_t depositBits(uint64_t x, uint64_t msk) { return _pdep_u64(x, msk); }
#else
// This is a naive implementation for CPUs that don't support BMI2
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

#endif  // SOF_UTIL_BIT_INCLUDED
