#ifndef BIT_H_INCLUDED
#define BIT_H_INCLUDED

#include <cstdint>

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

}  // namespace SoFUtil

#endif  // BIT_H_INCLUDED
