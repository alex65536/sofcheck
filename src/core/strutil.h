#ifndef SOF_CORE_STRUTIL_INCLUDED
#define SOF_CORE_STRUTIL_INCLUDED

#include <string>

#include "core/move.h"
#include "core/types.h"

namespace SoFCore {

inline constexpr char cellToChar(cell_t cell) {
  char transpos[] = ".PKNBRQ??pknbrq?";
  if (cell > 16) {
    return '?';
  }
  return transpos[cell];
}

inline constexpr const char *cellToUtf8(cell_t cell) {
  const char *transpos[] = {".", "♙", "♔", "♘", "♗", "♖", "♕", "?",
                            "?", "♟", "♚", "♞", "♝", "♜", "♛", "?"};
  if (cell > 16) {
    return "?";
  }
  return transpos[cell];
}

inline constexpr bool isValidXChar(char c) { return '1' <= c && c <= '8'; }
inline constexpr bool isValidYChar(char c) { return 'a' <= c && c <= 'h'; }

inline constexpr char xSubToChar(subcoord_t x) { return static_cast<char>('8' - x); }
inline constexpr char ySubToChar(subcoord_t y) { return static_cast<char>('a' + y); }

// If '1' <= c && c <= '8' doesn't hold, the behavior is undefined
inline constexpr subcoord_t charToSubX(char c) { return '8' - c; }

// If 'a' <= c && c <= 'h' doesn't hold, the behavior is undefined
inline constexpr subcoord_t charToSubY(char c) { return c - 'a'; }

// Recommended buffer size for moveToStr()
constexpr size_t BUFSZ_MOVE_STR = 6;

void moveToStr(Move move, char *str);
std::string moveToStr(Move move);

}  // namespace SoFCore

#endif  // SOF_CORE_STRUTIL_INCLUDED
