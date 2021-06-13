// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

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
