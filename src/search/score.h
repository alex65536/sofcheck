#ifndef SOF_SEARCH_SCORE_INCLUDED
#define SOF_SEARCH_SCORE_INCLUDED

#include <cstdint>

#include "bot_api/types.h"

namespace SoFSearch {

// Position score. The meaning of the position score depends on the value:
// * `score < -SCORE_CHECKMATE`: invalid
// * `-SCORE_CHECKMATE <= score <= -SCORE_CHECKMATE_THRESHOLD`: current side is checkmated
// * `-SCORE_CHECKMATE_THRESHOLD < score < SCORE_CHECKMATE_THRESHOLD`: normal score in centipawns
// * `SCORE_CHECKMATE_THRESHOLD <= score <= SCORE_CHECKMATE`: current side checkmates
// * `SCORE_CHECKMATE < score`: invalid
using score_t = int16_t;

// Infinite score. Note that we need to ensure that `-SCORE_INF` also fits in `score_t`
constexpr score_t SCORE_INF = 32767;

// Threshold score to indicate checkmate. If `abs(score) >= SCORE_CHECKMATE_THRESHOLD`, then it's
// checkmate score
constexpr score_t SCORE_CHECKMATE_THRESHOLD = 28000;

// Score for checkmate. This works as follows:
// * `p - SCORE_CHECKMATE` if the current side gets checkmate in `p` plies (`p` is even)
// * `SCORE_CHECKMATE - p` if the current side gives checkmate in `p` plies (`p` is odd)
constexpr score_t SCORE_CHECKMATE = 30000;

// Returns the score that corresponds to the current side getting checkmated in `p` plies. `p` must
// be even
inline constexpr score_t scoreCheckmateLose(const int16_t plies) { return plies - SCORE_CHECKMATE; }

// Returns the score that corresponds to the current side checkmating in `p` plies. `p` must be odd
inline constexpr score_t scoreCheckmateWin(const int16_t plies) { return SCORE_CHECKMATE - plies; }

// Returns `true` if the given score denotes a checkmate
inline constexpr bool isScoreCheckmate(const score_t score) {
  return score <= -SCORE_CHECKMATE_THRESHOLD || score >= SCORE_CHECKMATE_THRESHOLD;
}

// Returns `true` if `score` has a valid value. Note that `SCORE_INF` is just a theoretical bound
// and is considered invalid.
inline constexpr bool isScoreValid(const score_t score) {
  if (score > SCORE_CHECKMATE || score < -SCORE_CHECKMATE) {
    return false;
  }
  if (score <= -SCORE_CHECKMATE_THRESHOLD) {
    // We get checkmated, so the number of plies must be even
    return (score + SCORE_CHECKMATE) % 2 == 0;
  }
  if (score >= SCORE_CHECKMATE_THRESHOLD) {
    // We checkmate, so the number of plies must be odd
    return (SCORE_CHECKMATE - score) % 2 != 0;
  }
  return true;
}

// Adjusts the score as follows:
// - if the score denotes a checkmate, then `delta` is added to the number of plies before checkmate
// - otherwise, returns the initial value unchanged
inline constexpr score_t adjustCheckmate(const score_t score, int16_t delta) {
  if (score >= SCORE_CHECKMATE_THRESHOLD) {
    delta *= -1;
  } else if (score > -SCORE_CHECKMATE_THRESHOLD) {
    delta = 0;
  }
  return score + delta;
}

// The score must be valid here, otherwise the behaviour is undefined
inline constexpr SoFBotApi::PositionCost scoreToPositionCost(const score_t score) {
  using SoFBotApi::PositionCost;
  if (score <= -SCORE_CHECKMATE_THRESHOLD) {
    return PositionCost::checkMate(-((score + SCORE_CHECKMATE) >> 1));
  }
  if (score >= SCORE_CHECKMATE_THRESHOLD) {
    return PositionCost::checkMate((SCORE_CHECKMATE - score + 1) >> 1);
  }
  return PositionCost::centipawns(score);
}

// Pair of score values. Used to update middlegame score and endgame score at the same time
using score_pair_t = int32_t;

// Creates a score pair from two scores
inline constexpr score_pair_t makeScorePair(const score_t first, const score_t second) {
  return static_cast<score_pair_t>(second) * 0x10000 + static_cast<score_pair_t>(first);
}

// Creates a score pair from two equal scores
inline constexpr score_pair_t makeScorePair(const score_t score) {
  return makeScorePair(score, score);
}

// Extracts first item from the score pair
inline constexpr score_t scorePairFirst(const score_pair_t pair) {
  const uint16_t unsignedRes = static_cast<uint32_t>(pair) & 0xffffU;
  return static_cast<score_t>(unsignedRes);
}

// Extracts second item from the score pair
inline constexpr score_t scorePairSecond(const score_pair_t pair) {
  uint16_t unsignedRes = static_cast<uint32_t>(pair) >> 16;
  if (scorePairFirst(pair) < 0) {
    ++unsignedRes;
  }
  return static_cast<score_t>(unsignedRes);
}

// Compile-time tests for score pairs
static_assert(scorePairFirst(makeScorePair(1000, 8000)) == 1000);
static_assert(scorePairFirst(makeScorePair(1000, -8000)) == 1000);
static_assert(scorePairFirst(makeScorePair(-1000, 8000)) == -1000);
static_assert(scorePairFirst(makeScorePair(-1000, -8000)) == -1000);
static_assert(scorePairSecond(makeScorePair(1000, 8000)) == 8000);
static_assert(scorePairSecond(makeScorePair(1000, -8000)) == -8000);
static_assert(scorePairSecond(makeScorePair(-1000, 8000)) == 8000);
static_assert(scorePairSecond(makeScorePair(-1000, -8000)) == -8000);

}  // namespace SoFSearch

#endif  // SOF_SEARCH_SCORE_INCLUDED
