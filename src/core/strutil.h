#ifndef STRUTIL_H_INCLUDED
#define STRUTIL_H_INCLUDED

#include "core/types.h"

namespace SoFCore {

inline constexpr char cellToChar(cell_t cell) {
  if (cell == INVALID_CELL) {
    return '?';
  } 
  char transpos[] = ".PKNBRQ%%pknbrq%";
  if (cell > 16) {
    return '%';
  }
  return transpos[cell];
}

inline constexpr bool isValidXChar(char c) { return '1' <= c && c <= '8'; }
inline constexpr bool isValidYChar(char c) { return 'a' <= c && c <= 'h'; }

inline constexpr char xSubToChar(subcoord_t x) { return '8' - x; }
inline constexpr char ySubToChar(subcoord_t y) { return 'a' + y; }

// If '1' <= c && c <= '8' doesn't hold, the behavior is undefined
inline constexpr subcoord_t charToSubX(char c) { return '8' - c; }

// If 'a' <= c && c <= 'h' doesn't hold, the behavior is undefined
inline constexpr subcoord_t charToSubY(char c) { return c - 'a'; }

}  // namespace SoFCore

#endif  // STRUTIL_H_INCLUDED
