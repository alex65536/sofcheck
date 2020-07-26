#ifndef SOF_CORE_STRUTIL_INCLUDED
#define SOF_CORE_STRUTIL_INCLUDED

#include <string>

#include "core/board.h"
#include "core/move.h"
#include "core/types.h"
#include "util/misc.h"

namespace SoFCore {

inline constexpr char cellToChar(cell_t cell) {
  constexpr char transpos[] = ".PKNBRQ??pknbrq?";
  if (SOF_UNLIKELY(cell > 16)) {
    return '?';
  }
  return transpos[cell];
}

inline constexpr const char *cellToUtf8(cell_t cell) {
  constexpr const char *transpos[] = {".", "♙", "♔", "♘", "♗", "♖", "♕", "?",
                                      "?", "♟", "♚", "♞", "♝", "♜", "♛", "?"};
  if (SOF_UNLIKELY(cell > 16)) {
    return "?";
  }
  return transpos[cell];
}

// Returns true if `c` is a valid character representation of X subcoordinate
inline constexpr bool isXCharValid(char c) { return '1' <= c && c <= '8'; }

// Returns true if `c` is a valid character representation of Y subcoordinate
inline constexpr bool isYCharValid(char c) { return 'a' <= c && c <= 'h'; }

// Returns character representation of X subcoordinate
inline constexpr char xSubToChar(subcoord_t x) { return static_cast<char>('8' - x); }

// Returns character representation of Y subcoordinate
inline constexpr char ySubToChar(subcoord_t y) { return static_cast<char>('a' + y); }

// Returns X subcoordinate from its character representation `c`
//
// If `'1' <= c && c <= '8'` doesn't hold, the behavior is undefined
inline constexpr subcoord_t charToSubX(char c) { return '8' - c; }

// Returns Y subcoordinate from its character representation `c`
//
// If `'a' <= c && c <= 'h'` doesn't hold, the behavior is undefined
inline constexpr subcoord_t charToSubY(char c) { return c - 'a'; }

// Creates a coordinate from its string representation. If invalid string representation is given,
// the behaviour of this function is undefined.
//
// For example, `charsToCoord('c', '5')` returns coord_t that corresponds to c5 cell.
inline constexpr coord_t charsToCoord(char cy, char cx) {
  return makeCoord(charToSubX(cx), charToSubY(cy));
}

// Recommended buffer size for `moveToStr()`
constexpr size_t BUFSZ_MOVE_STR = 6;

void moveToStr(Move move, char *str);
std::string moveToStr(Move move);

const char *fenParseResultToStr(FenParseResult res);
const char *validateResultToStr(ValidateResult res);

}  // namespace SoFCore

#endif  // SOF_CORE_STRUTIL_INCLUDED
