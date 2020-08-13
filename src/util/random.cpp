#include "util/random.h"

#include <chrono>
#include <random>
#include <thread>

#include "config.h"
#include "util/misc.h"

namespace SoFUtil {

static uint64_t makeRandomSeed() {
  return std::chrono::steady_clock::now().time_since_epoch().count() +
         static_cast<uint64_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
}

struct RandGenWrapper {
  std::mt19937_64 gen;

  RandGenWrapper() noexcept : gen(makeRandomSeed()) {}
};

uint64_t random() {
  static thread_local RandGenWrapper wrapper;
  return wrapper.gen();
}

}  // namespace SoFUtil
