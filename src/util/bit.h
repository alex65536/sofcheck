#ifndef BIT_H_INCLUDED
#define BIT_H_INCLUDED

#include <cstdint>

namespace SoFUtil {

inline constexpr int popcount(uint64_t x) {
  return __builtin_popcountll(x);
}

}

#endif // BIT_H_INCLUDED
