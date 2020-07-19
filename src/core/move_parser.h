#ifndef SOF_CORE_MOVE_PARSER_INCLUDED
#define SOF_CORE_MOVE_PARSER_INCLUDED

#include <cstring>

#include "core/board.h"
#include "core/move.h"
#include "core/types.h"

namespace SoFCore {

struct ParsedMove {
  uint8_t unused;  // Padding field, shall be set to zero
  coord_t src;
  coord_t dst;
  cell_t promote;  // This field holds only EMPTY_CELL or a white piece. If, for example, the move
                   // is a promote to black queen, this fields holds white queen.

  // Returns ParsedMove from its UCI string representation. `first` is the pointer to the beginning
  // of the string, and `last` is the pointer past the end of the string. If the given string cannot
  // be interpreted as `ParsedMove`, the return value is equal to `INVALID_PARSED_MOVE`
  static ParsedMove fromStr(const char *first, const char *last);

  // Variation of `fromStr` that works with zero-terminated strings
  inline static ParsedMove fromStr(const char *str) { return fromStr(str, str + std::strlen(str)); }

  // Serializes the structure into `uint32_t`
  //
  // The compiler should optimize this and just reinterpet the structure as `uint32_t` for
  // little-endian architectures. The engine is not optimized for big-endian now, so there is no
  // fast implementation for such architectures at the moment.
  inline constexpr uint32_t asUint() const {
    return static_cast<uint32_t>(unused) | (static_cast<uint32_t>(src) << 8) |
           (static_cast<uint32_t>(dst) << 16) | (static_cast<uint32_t>(promote) << 24);
  }
};

constexpr ParsedMove INVALID_PARSED_MOVE{0, INVALID_COORD, INVALID_COORD, 0};

inline constexpr bool operator==(ParsedMove a, ParsedMove b) { return a.asUint() == b.asUint(); }

inline constexpr bool operator!=(ParsedMove a, ParsedMove b) { return a.asUint() != b.asUint(); }

// Converts `parsedMove` into `Move`, assuming that it is applied from position `board`. The
// returned move is not guaranteed to be pseudo-legal or well-formed, use `move.isWellFormed()` and
// `isMoveValid()` to ensure this.
//
// In case where `parsedMove` cannot be interpreted as `Move`, it returns the move with `kind ==
// MoveKind::Invalid`.
Move moveFromParsed(const ParsedMove parsedMove, const Board &board);

// Convenience function that converts the move directly from string. For details, see the
// implementation of this function and the documentation for `ParsedMove::fromStr` and
// `moveFromParsed`.
inline Move moveParse(const char *first, const char *last, const Board &board) {
  const ParsedMove parsed = ParsedMove::fromStr(first, last);
  return moveFromParsed(parsed, board);
}

// Variation of `moveParse` that works with zero-terminated strings
inline Move moveParse(const char *str, const Board &board) {
  const ParsedMove parsed = ParsedMove::fromStr(str);
  return moveFromParsed(parsed, board);
}

}  // namespace SoFCore

#endif  // SOF_CORE_MOVE_PARSER_INCLUDED
