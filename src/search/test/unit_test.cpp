#include <gtest/gtest.h>

#include <algorithm>
#include <limits>

#include "search/private/score.h"

TEST(SoFSearch, ScorePair) {
  using namespace SoFSearch::Private;

  static constexpr score_t SCORE_MIN = std::numeric_limits<score_t>::min();
  static constexpr score_t SCORE_MAX = std::numeric_limits<score_t>::max();
  static constexpr score_t TEST_SCORES[] = {
      SCORE_MIN, SCORE_MIN + 1, SCORE_MIN + 2, SCORE_MIN + 3, -3,       -2, -1, 0, 1, 2,
      3,         SCORE_MAX - 3, SCORE_MAX - 2, SCORE_MAX - 1, SCORE_MAX};

  for (score_t first : TEST_SCORES) {
    for (score_t second : TEST_SCORES) {
      const score_pair_t pair = makeScorePair(first, second);
      EXPECT_EQ(scorePairFirst(pair), first);
      EXPECT_EQ(scorePairSecond(pair), second);
      if (first != SCORE_MIN && second != SCORE_MIN) {
        EXPECT_EQ(-scorePairFirst(pair), -first);
        EXPECT_EQ(-scorePairSecond(pair), -second);
      }
      if (std::abs(first) <= 3 && std::abs(second) <= 3) {
        EXPECT_EQ(2 * scorePairFirst(pair), 2 * first);
        EXPECT_EQ(4 * scorePairFirst(pair), 4 * first);
        EXPECT_EQ(41 * scorePairFirst(pair), 41 * first);
        EXPECT_EQ(2 * scorePairSecond(pair), 2 * second);
        EXPECT_EQ(4 * scorePairSecond(pair), 4 * second);
        EXPECT_EQ(41 * scorePairSecond(pair), 41 * second);
      }
    }
  }

  for (score_t first1 = -5; first1 <= 5; ++first1) {
    for (score_t first2 = -5; first2 <= 5; ++first2) {
      for (score_t second1 = -5; second1 <= 5; ++second1) {
        for (score_t second2 = -5; second2 <= 5; ++second2) {
          const score_pair_t pair1 = makeScorePair(first1, second1);
          const score_pair_t pair2 = makeScorePair(first2, second2);
          EXPECT_EQ(scorePairFirst(pair1 + pair2), first1 + first2);
          EXPECT_EQ(scorePairSecond(pair1 + pair2), second1 + second2);
          EXPECT_EQ(scorePairFirst(pair1 - pair2), first1 - first2);
          EXPECT_EQ(scorePairSecond(pair1 - pair2), second1 - second2);
        }
      }
    }
  }
}
