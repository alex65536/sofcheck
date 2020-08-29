#include "util/parallel.h"

#include <algorithm>
#include <thread>
#include <vector>

namespace SoFUtil {

void processSegmentParallel(const size_t left, const size_t right, size_t jobs,
                            const std::function<void(size_t, size_t)> &func) {
  if (left >= right) {
    return;
  }
  const size_t distance = right - left;

  // We don't add threads that don't process any elements
  jobs = std::min(jobs, distance);

  if (jobs == 1) {
    func(left, right);
    return;
  }

  const size_t blockSize = distance / jobs;
  const size_t blockRemainder = distance % jobs;
  size_t curLeft = left;
  std::vector<std::thread> threads(jobs);

  for (size_t i = 0; i < jobs; ++i) {
    size_t curSize = blockSize;
    if (i < blockRemainder) {
      ++curSize;
    }
    const size_t curRight = curLeft + curSize;
    threads[i] = std::thread(func, curLeft, curRight);
    curLeft = curRight;
  }

  for (std::thread &thread : threads) {
    thread.join();
  }
}

}  // namespace SoFUtil
