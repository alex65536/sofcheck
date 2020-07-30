#ifndef SOF_CORE_MOVE_PARSER_INCLUDED
#define SOF_CORE_MOVE_PARSER_INCLUDED

#include <cstring>

#include "core/board.h"
#include "core/move.h"
#include "core/types.h"

namespace SoFCore {

// The piece to promote the pawn. Set to `None` if the move is not a promotion.
enum class PromotePiece : int8_t { None = 0, Knight = 1, Bishop = 2, Rook = 3, Queen = 4 };

struct ParsedMove {
  PromotePiece promote;
  coord_t src;
  coord_t dst;
  uint8_t unused;  // Padding field, shall be set to zero

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
    const auto uintPromote = static_cast<uint8_t>(promote);
    const auto uintSrc = static_cast<uint8_t>(src);
    const auto uintDst = static_cast<uint8_t>(dst);
    return static_cast<uint32_t>(uintPromote) | (static_cast<uint32_t>(uintSrc) << 8) |
           (static_cast<uint32_t>(uintDst) << 16) | (static_cast<uint32_t>(unused) << 24);
  }
};

constexpr ParsedMove INVALID_PARSED_MOVE{PromotePiece::None, INVALID_COORD, INVALID_COORD, 0};

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
