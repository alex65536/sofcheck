// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

#ifndef SELFTEST_UTIL_H_INCLUDED
#define SELFTEST_UTIL_H_INCLUDED

#include <cassert>

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

#endif  // SELFTEST_UTIL_H_INCLUDED
