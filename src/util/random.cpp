#include "util/random.h"

#include <chrono>
#include <random>

namespace SoFUtil {

static std::mt19937_64 g_randGen(std::chrono::steady_clock::now().time_since_epoch().count());

uint64_t random() { return g_randGen(); }

}  // namespace SoFUtil
