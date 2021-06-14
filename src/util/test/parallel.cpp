// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <mutex>
#include <vector>

#include "util/parallel.h"

void checkParallel(const size_t left, const size_t right, const size_t jobs) {
  std::vector<std::pair<size_t, size_t>> segments;
  std::mutex mutex;
  SoFUtil::processSegmentParallel(left, right, jobs,
                                  [&segments, &mutex](const size_t l, const size_t r) {
                                    std::lock_guard lock(mutex);
                                    segments.emplace_back(l, r);
                                  });
  std::sort(segments.begin(), segments.end());
  ASSERT_FALSE(segments.empty());
  ASSERT_EQ(segments[0].first, left);
  ASSERT_EQ(segments.back().second, right);
  for (const auto &[curLeft, curRight] : segments) {
    ASSERT_LT(curLeft, curRight);
  }
  for (size_t i = 1; i < segments.size(); ++i) {
    ASSERT_EQ(segments[i - 1].second, segments[i].first);
  }
}

TEST(SoFUtil, ProcessSegmentParallel) {
  constexpr size_t maxVal = std::numeric_limits<size_t>::max();
  for (size_t jobs = 1; jobs <= 16; ++jobs) {
    checkParallel(0, 5, jobs);
    checkParallel(1, 6, jobs);
    checkParallel(0, 997, jobs);
    checkParallel(0, 998, jobs);
    checkParallel(0, 1440, jobs);
    checkParallel(0, 1441, jobs);
    checkParallel(1440, 2880, jobs);
    checkParallel(1440, 2881, jobs);
    checkParallel(maxVal - 1, maxVal, jobs);
    checkParallel(maxVal - 2, maxVal, jobs);
    checkParallel(maxVal - 3, maxVal, jobs);
    checkParallel(maxVal - 4, maxVal, jobs);
    checkParallel(0, maxVal, jobs);
    checkParallel(0, maxVal - 1, jobs);
    checkParallel(0, maxVal - 2, jobs);
    checkParallel(0, 1, jobs);
  }
}
