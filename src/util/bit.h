#ifndef BIT_H_INCLUDED
#define BIT_H_INCLUDED

#include <cstdint>

namespace SoFUtil {

// Returns the number of ones in x
inline constexpr int popcount(uint64_t x) { return __builtin_popcountll(x); }

// Clears the lowest bit set to one
inline constexpr uint64_t clearLowest(uint64_t x) { return x & (x - 1); }

}  // namespace SoFUtil

#endif  // BIT_H_INCLUDED
