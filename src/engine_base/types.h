#ifndef SOF_ENGINE_BASE_TYPES_INCLUDED
#define SOF_ENGINE_BASE_TYPES_INCLUDED

#include <chrono>
#include <cstdint>
#include <limits>

#include "core/types.h"

namespace SoFEngineBase {

// Result of client or server API calls. This is only the error codes, the error messages (if any)
// must be reported separately via `reportError()`
enum class ApiResult {
  // API call was successful
  Ok,
  // API call is not implemented
  NotSupported,
  // Invalid arguments passed to API call
  InvalidArgument,
  // API methods are called in invalid order (e.g. `stop()` without `search...()` before it)
  UnexpectedCall,
  // Error due to bug in the code which implements this API
  ApiError,
  // Input/output error
  IOError,
  // Unknown error not listed here. More details may be passed with an error message
  RuntimeError
};

// TODO : move these types into other package (and call it smth like core_extra)

using std::chrono::milliseconds;

constexpr size_t INFINITE_MOVES = static_cast<size_t>(-1);

// Time control for one side
struct TimeControlSide {
  milliseconds time;  // Time left on the clock (or `milliseconds::max()` if unset)
  milliseconds inc;   // Time added after each move
};

// Time control for both sides
struct TimeControl {
  TimeControlSide white;
  TimeControlSide black;
  size_t movesToGo;  // Positive number that represents the number of moves until time control
                     // changes. If it doesn't change, the value is equal to `INFINITE_MOVES`

  inline constexpr TimeControlSide &operator[](SoFCore::Color color) {
    return (color == SoFCore::Color::White) ? white : black;
  }

  inline constexpr const TimeControlSide &operator[](SoFCore::Color color) const {
    return (color == SoFCore::Color::White) ? white : black;
  }
};

// The type denoting if the position cost is exact, or it is a lower/upper bound
enum class PositionCostBound : uint8_t { Exact, Lowerbound, Upperbound };

// Type of the position cost
enum class PositionCostType {
  Centipawns,  // The postion cost is stored in centipawns
  Checkmate    // The position cost is stored in moves until checkmate
};

// Minimum and maximum allowed values (in centipawns) for position cost
constexpr int32_t MIN_POSITION_COST = -2'000'000'000;
constexpr int32_t MAX_POSITION_COST = 2'000'000'000;

// Structure that represents the position cost
struct PositionCost {
public:
  inline constexpr PositionCost() : value_(0) {}

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
    return (value_ < 0) ? (VALUE_MIN - value_) : (VALUE_MAX - value_ + 1);
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

  int32_t value_;
};

// The type that indicates number of permille
using permille_t = uint16_t;

constexpr permille_t PERMILLE_UNKNOWN = static_cast<permille_t>(-1);

}  // namespace SoFEngineBase

#endif  // SOF_ENGINE_BASE_TYPES_INCLUDED