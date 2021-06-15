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

#ifndef SOF_SEARCH_PRIVATE_LIMITS_INCLUDED
#define SOF_SEARCH_PRIVATE_LIMITS_INCLUDED

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "bot_api/types.h"

namespace SoFCore {
struct Board;
}  // namespace SoFCore

namespace SoFSearch::Private {

constexpr size_t DEPTH_UNLIMITED = std::numeric_limits<size_t>::max();
constexpr uint64_t NODES_UNLIMITED = std::numeric_limits<uint64_t>::max();
constexpr std::chrono::milliseconds TIME_UNLIMITED = std::chrono::milliseconds::max();

struct SearchLimits {
  // Maximum depth (or `DEPTH_UNLIMITED` if unlimited)
  size_t depth = DEPTH_UNLIMITED;
  // Maximum nodes (or `NODES_UNLIMITED` if unlimited)
  uint64_t nodes = NODES_UNLIMITED;
  // Maximum time (or `TIME_UNLIMITED` if unlimited)
  std::chrono::milliseconds time = TIME_UNLIMITED;
  // Time control (default-constructed if not present)
  SoFBotApi::TimeControl timeControl;

  // Constructs `SearchLimits` with infinite time
  inline static SearchLimits withInfiniteTime() { return SearchLimits{}; }

  // Constructs `SearchLimits` for fixed depth
  inline static SearchLimits withFixedDepth(const size_t depth) {
    return SearchLimits{depth, NODES_UNLIMITED, TIME_UNLIMITED, SoFBotApi::TimeControl{}};
  }

  // Constructs `SearchLimits` for fixed nodes
  inline static SearchLimits withFixedNodes(const uint64_t nodes) {
    return SearchLimits{DEPTH_UNLIMITED, nodes, TIME_UNLIMITED, SoFBotApi::TimeControl{}};
  }

  // Constructs `SearchLimits` for fixed time
  inline static SearchLimits withFixedTime(const std::chrono::milliseconds time) {
    return SearchLimits{DEPTH_UNLIMITED, NODES_UNLIMITED, time, SoFBotApi::TimeControl{}};
  }

  // Constructs `SearchLimits` for given time control. This function also determines thinking time
  // based on the given time control.
  static SearchLimits withTimeControl(const SoFCore::Board &board,
                                      const SoFBotApi::TimeControl &timeControl);
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_LIMITS_INCLUDED
