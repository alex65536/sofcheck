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

#ifndef SOF_BOT_API_TYPES_INCLUDED
#define SOF_BOT_API_TYPES_INCLUDED

#include <chrono>
#include <cstdint>
#include <limits>

#include "core/move.h"
#include "core/types.h"

namespace SoFBotApi {

constexpr size_t MOVES_INFINITE = std::numeric_limits<size_t>::max();

// Time control for one side
struct TimeControlSide {
  std::chrono::milliseconds time;  // Time left on the clock (or `milliseconds::max()` if unset)
  std::chrono::milliseconds inc;   // Time added after each move

  inline constexpr TimeControlSide() : time(std::chrono::milliseconds::max()), inc(0) {}
};

// Time control for both sides
struct TimeControl {
  TimeControlSide white;
  TimeControlSide black;
  size_t movesToGo;  // Positive number that represents the number of moves until time control
                     // changes. If it doesn't change, the value is equal to `MOVES_INFINITE`

  inline constexpr TimeControl() : movesToGo(MOVES_INFINITE) {}

  inline constexpr TimeControlSide &operator[](SoFCore::Color color) {
    return (color == SoFCore::Color::White) ? white : black;
  }

  inline constexpr const TimeControlSide &operator[](SoFCore::Color color) const {
    return (color == SoFCore::Color::White) ? white : black;
  }
};

// The type denoting if the position cost is exact, or it is a lower/upper bound
enum class PositionCostBound : uint8_t { Exact = 0, Lowerbound = 1, Upperbound = 2 };

// Type of the position cost
enum class PositionCostType {
  Centipawns,  // The postion cost is stored in centipawns
  Checkmate    // The position cost is stored in moves until checkmate
};

// Minimum and maximum allowed values (in centipawns) for `PositionCost`
constexpr int32_t MIN_POSITION_COST = -2'000'000'000;
constexpr int32_t MAX_POSITION_COST = 2'000'000'000;

// Structure that represents the position cost
struct PositionCost {
public:
  inline constexpr PositionCost() = default;

  // Returns the position cost type
  inline constexpr PositionCostType type() const {
    return (MIN_POSITION_COST <= value_ && value_ <= MAX_POSITION_COST)
               ? PositionCostType::Centipawns
               : PositionCostType::Checkmate;
  }

  // Creates a position cost in centipawns. The greater the parameter, the better the cost for
  // current moving side.
  //
  // The parameter `cp` must be in range from `MIN_POSITION_COST` to `MAX_POSITION_COST`, otherwise
  // the behaviour is undefined.
  inline static constexpr PositionCost centipawns(int32_t cp) { return PositionCost(cp); }

  // Create a position in moves (not plies!) until checkmate. Depending on value, `moves` denotes
  // the following:
  // - if `moves <= 0`, then current moving side receives checkmate in `-moves` moves
  // - if `moves > 0`, then current moving side can checkmate in `moves` moves
  inline static constexpr PositionCost checkMate(int16_t moves) {
    const int32_t value = (moves <= 0) ? (VALUE_MIN - moves) : (VALUE_MAX - (moves - 1));
    return PositionCost(value);
  }

  // Returns the position cost in centipawns. If `type()` is not equal to `Centipawns`, then the
  // behaviour is undefined
  inline constexpr int32_t centipawns() const { return value_; }

  // Returns the number of moves until checkmate in the format described above. If `type()` is not
  // equal to `Centipawns`, then the behaviour is undefined
  inline constexpr int16_t checkMate() const {
    return static_cast<uint16_t>((value_ < 0) ? (VALUE_MIN - value_) : (VALUE_MAX - value_ + 1));
  }

  // Comparison operators
#define D_ADD_COMPARE(op)                                                    \
  inline constexpr friend bool operator op(PositionCost a, PositionCost b) { \
    return a.value_ op b.value_;                                             \
  }
  D_ADD_COMPARE(==);
  D_ADD_COMPARE(!=);
  D_ADD_COMPARE(<);
  D_ADD_COMPARE(<=);
  D_ADD_COMPARE(>);
  D_ADD_COMPARE(>=);
#undef D_ADD_COMPARE

private:
  constexpr static int32_t VALUE_MIN = std::numeric_limits<int32_t>::min();
  constexpr static int32_t VALUE_MAX = std::numeric_limits<int32_t>::max();

  inline explicit constexpr PositionCost(int32_t value) : value_(value) {}

  int32_t value_ = 0;
};

// The type that indicates number of permille
using permille_t = uint16_t;

// Intermediate search result
struct SearchResult {
  size_t depth;             // Search depth (in plies)
  const SoFCore::Move *pv;  // The best line found
  size_t pvLen;             // Length of the best line found (if not present, set to zero)
  PositionCost cost;        // Estimated position cost
  PositionCostBound bound;  // Is position cost exact?
};

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_TYPES_INCLUDED
