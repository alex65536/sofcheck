#include "util/random.h"

#include <chrono>
#include <random>

#include "util/misc.h"

namespace SoFUtil {

// NOLINTNEXTLINE(cert-err58-cpp): possible exception is caught, but clang-tidy doesn't understand
static std::mt19937_64 g_randGen = []() noexcept {
  // Fix cert-err58-cpp, handle the possible exception during static initialization
  try {
    return std::mt19937_64(std::chrono::steady_clock::now().time_since_epoch().count());
  } catch (...) {
    panic("Error initializing g_randGen");
  }
}();

uint64_t random() { return g_randGen(); }

}  // namespace SoFUtil
