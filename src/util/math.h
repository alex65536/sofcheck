#ifndef SOF_UTIL_MATH_INCLUDED
#define SOF_UTIL_MATH_INCLUDED

#include <cstddef>
#include <cstdint>

namespace SoFUtil {

// Computes `floor(log10(val))` where `val` is a positive number
inline constexpr size_t log10(uint64_t val) {
  // Stub to prevent integer overflow
  if (val >= static_cast<uint64_t>(1e19)) {
    return 19;
  }

  uint64_t res = 0;
  uint64_t pow = 1;
  while (pow <= val) {
    pow *= 10;
    ++res;
  }
  return res - 1;
}

}  // namespace SoFUtil

#endif  // SOF_UTIL_MATH_INCLUDED
