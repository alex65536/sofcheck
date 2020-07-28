#include "util/random.h"

#include <chrono>
#include <random>

#include "config.h"
#include "util/misc.h"

namespace SoFUtil {

// NOLINTNEXTLINE(cert-err58-cpp): possible exception is caught, but clang-tidy doesn't understand
static std::mt19937_64 g_randGen = []() noexcept {
// Fix cert-err58-cpp, handle the possible exception during static initialization
#ifndef USE_NO_EXCEPTIONS
  try {
#endif
    return std::mt19937_64(std::chrono::steady_clock::now().time_since_epoch().count());
#ifndef USE_NO_EXCEPTIONS
  } catch (...) {
    panic("Error initializing g_randGen");
  }
#endif
}();

uint64_t random() { return g_randGen(); }

}  // namespace SoFUtil
