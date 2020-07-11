#include "strutil.h"

#include <cstring>

namespace SoFCore {

std::string moveToStr(const Move move) {
  char str[6];
  moveToStr(move, str);
  return std::string(str);
}

void moveToStr(const SoFCore::Move move, char *str) {
  if (move.kind == MoveKind::Null) {
    strcpy(str, "0000");
    return;
  }
  *(str++) = ySubToChar(coordY(move.src));
  *(str++) = xSubToChar(coordX(move.src));
  *(str++) = ySubToChar(coordY(move.dst));
  *(str++) = xSubToChar(coordX(move.dst));
  if (move.kind == MoveKind::Promote) {
    const char transpos[] = ".pknbrq?";
    *(str++) = transpos[move.promote & 7];
  }
  *str = '\0';
}

}  // namespace SoFCore
