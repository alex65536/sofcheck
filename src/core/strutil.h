#ifndef STRUTIL_H_INCLUDED
#define STRUTIL_H_INCLUDED

namespace SoFCore {

#include "core/types.h"

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

inline constexpr char xSubToChar(subcoord_t x) { return '8' - x; }
inline constexpr char ySubToChar(subcoord_t y) { return 'a' + y; }

}  // namespace SoFCore

#endif  // STRUTIL_H_INCLUDED
