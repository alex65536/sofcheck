// This file is part of SoFCheck
//
// Copyright (c) 2020-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#include "search/private/limits.h"

#include <algorithm>

#include "core/board.h"
#include "util/misc.h"

namespace SoFSearch::Private {

using namespace std::chrono_literals;

using std::chrono::milliseconds;

constexpr milliseconds HARD_TIME_MARGIN_PER_MOVE = 3ms;
constexpr milliseconds HARD_TIME_MARGIN = 250ms;
constexpr milliseconds SOFT_TIME_MARGIN_PER_MOVE = 5ms;
constexpr milliseconds SOFT_TIME_MARGIN = 350ms;
constexpr milliseconds MIN_TIME_MARGIN = 20ms;

constexpr int64_t MAX_MOVES_LEFT = 50;
constexpr int64_t MAX_MOVES_TO_GO = 1000;

inline static milliseconds doCalculateMaxTime(const SoFCore::Board &board,
                                              const milliseconds totalTime, const int64_t movesToGo,
                                              const milliseconds margin) {
  int64_t movesLeft = std::min(MAX_MOVES_LEFT, movesToGo);
  SOF_ASSERT(movesLeft > 0);
  if (board.moveNumber < 10) {
    // Don't think too much on first moves
    movesLeft *= 2;
  }
  return std::max(2ms, (totalTime - margin) / movesLeft);
}

inline static milliseconds calculateMaxTime(const SoFCore::Board &board,
                                            const SoFBotApi::TimeControl &timeControl) {
  // Inspect time control
  milliseconds totalTime = timeControl[board.side].time;
  const milliseconds inc = timeControl[board.side].inc;
  const auto movesToGo =
      (timeControl.movesToGo == SoFBotApi::MOVES_INFINITE)
          ? MAX_MOVES_TO_GO
          : std::min(MAX_MOVES_TO_GO, static_cast<int64_t>(timeControl.movesToGo));
  if (totalTime == milliseconds::max()) {
    // This happens when the UCI client didn't set time properly. I don't know whether such cases
    // are valid, but it's better to come up with some definite value other than infinity.
    totalTime = 1h;
  }

  // Calculate margins
  const auto hardMargin =
      std::min(HARD_TIME_MARGIN, MIN_TIME_MARGIN + movesToGo * HARD_TIME_MARGIN_PER_MOVE);
  const auto softMargin =
      std::min(SOFT_TIME_MARGIN, MIN_TIME_MARGIN + movesToGo * SOFT_TIME_MARGIN_PER_MOVE);

  // Safeguards against time forfeit
  if (totalTime <= hardMargin) {
    return 1ms;
  }
  if (totalTime <= softMargin) {
    return 2ms;
  }

  // Calculate maximum time for thinking
  milliseconds time = doCalculateMaxTime(board, totalTime, movesToGo, softMargin) + inc;

  // More safeguards against time forfeit
  time = std::min(time, totalTime - hardMargin);

  return std::max(time, 2ms);
}

SearchLimits SearchLimits::withTimeControl(const SoFCore::Board &board,
                                           const SoFBotApi::TimeControl &timeControl) {
  const milliseconds maxTime = calculateMaxTime(board, timeControl);
  return SearchLimits{DEPTH_UNLIMITED, NODES_UNLIMITED, maxTime, timeControl};
}

}  // namespace SoFSearch::Private
