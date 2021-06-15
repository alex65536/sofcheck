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

#include "eval/score.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <limits>

TEST(SoFEval, ScorePair) {
  using namespace SoFEval;

  static constexpr score_t SCORE_MIN = std::numeric_limits<score_t>::min();
  static constexpr score_t SCORE_MAX = std::numeric_limits<score_t>::max();
  static constexpr score_t TEST_SCORES[] = {
      SCORE_MIN, SCORE_MIN + 1, SCORE_MIN + 2, SCORE_MIN + 3, -3,       -2, -1, 0, 1, 2,
      3,         SCORE_MAX - 3, SCORE_MAX - 2, SCORE_MAX - 1, SCORE_MAX};

  for (score_t first : TEST_SCORES) {
    for (score_t second : TEST_SCORES) {
      const auto pair = ScorePair::from(first, second);
      EXPECT_EQ(pair.first(), first);
      EXPECT_EQ(pair.second(), second);
      if (first != SCORE_MIN && second != SCORE_MIN) {
        EXPECT_EQ((-pair).first(), -first);
        EXPECT_EQ((-pair).second(), -second);
      }
      if (std::abs(first) <= 3 && std::abs(second) <= 3) {
        EXPECT_EQ((2 * pair).first(), 2 * first);
        EXPECT_EQ((4 * pair).first(), 4 * first);
        EXPECT_EQ((41 * pair).first(), 41 * first);
        EXPECT_EQ((2 * pair).second(), 2 * second);
        EXPECT_EQ((4 * pair).second(), 4 * second);
        EXPECT_EQ((41 * pair).second(), 41 * second);
      }
    }
  }

  for (score_t first1 = -5; first1 <= 5; ++first1) {
    for (score_t first2 = -5; first2 <= 5; ++first2) {
      for (score_t second1 = -5; second1 <= 5; ++second1) {
        for (score_t second2 = -5; second2 <= 5; ++second2) {
          const auto pair1 = ScorePair::from(first1, second1);
          const auto pair2 = ScorePair::from(first2, second2);
          EXPECT_EQ((pair1 + pair2).first(), first1 + first2);
          EXPECT_EQ((pair1 + pair2).second(), second1 + second2);
          EXPECT_EQ((pair1 - pair2).first(), first1 - first2);
          EXPECT_EQ((pair1 - pair2).second(), second1 - second2);
        }
      }
    }
  }
}
