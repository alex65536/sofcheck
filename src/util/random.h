#ifndef SOF_UTIL_RANDOM_INCLUDED
#define SOF_UTIL_RANDOM_INCLUDED

#include <cstdint>

namespace SoFUtil {

// Generates a random number.
//
// It is disregarded to use this function during static initialization.
uint64_t random();

}  // namespace SoFUtil

#endif  // SOF_UTIL_RANDOM_INCLUDED
