#ifndef SOF_UTIL_RANDOM_INCLUDED
#define SOF_UTIL_RANDOM_INCLUDED

#include <algorithm>
#include <cstdint>
#include <limits>

namespace SoFUtil {

// Generates a random number.
//
// It is disregarded to use this function during static initialization.
uint64_t random();

template <typename Iter>
void randomShuffle(Iter first, Iter last) {
  struct Random {
    using result_type = uint64_t;
    inline result_type operator()() { return random(); }
    inline result_type min() const { return std::numeric_limits<result_type>::min(); }
    inline result_type max() const { return std::numeric_limits<result_type>::max(); }
  };
  std::shuffle(first, last, Random{});
}

}  // namespace SoFUtil

#endif  // SOF_UTIL_RANDOM_INCLUDED
