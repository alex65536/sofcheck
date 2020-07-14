#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "chess_intf.h"

namespace ChessIntf {

inline int getMoveHash(const Board &board, const Move &move) {
  char str[6];
  moveStr(board, move, str);
  int res = (str[0] - 'a') * 512 + (str[1] - '1') * 64 + (str[2] - 'a') * 8 + (str[3] - '1');
  res *= 5;
  switch (str[4]) {
    case '\0':
      break;
    case 'n':
      res += 1;
      break;
    case 'b':
      res += 2;
      break;
    case 'r':
      res += 3;
      break;
    case 'q':
      res += 4;
      break;
    default:
      assert(false);
  }
  return res;
}

}  // namespace ChessIntf

#endif
