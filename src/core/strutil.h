#ifndef STRUTIL_H_INCLUDED
#define STRUTIL_H_INCLUDED

#include "core/types.h"
#include <cstdint>

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

// Gets the next token from str as uint32_t and puts the result into res
// Returns the number of chars read (or -1 in case of error)
int uintParse(uint32_t &res, const char *str);

// Writes res to the string str, including the terminating null character
// Returns the number of chars written
int uintSave(uint32_t val, char *str);

}  // namespace SoFCore

#endif  // STRUTIL_H_INCLUDED
