#include "strutil.h"

#include <cstring>
#include <string>
#include <limits>

namespace SoFCore {

int uintParse(uint32_t &res, const char *str) {
  uint64_t largeRes = 0;
  int chars = 0;
  while (true) {
    char c = *(str++);
    if (c == ' ' || c == '\t' || c == '\n' || c == '\0') {
      if (chars == 0 || largeRes > std::numeric_limits<uint32_t>::max()) {
        return -1;
      }
      res = largeRes;
      return chars;
    }
    ++chars;
    if (chars >= 11) {
      return -1;
    }
    if ('0' <= c && c <= '9') {
      largeRes *= 10;
      largeRes += c - '0';
      continue;
    }
    return -1;
  }
}

int uintSave(uint32_t val, char *str) {
  std::string res = std::to_string(val);
  std::strcpy(str, res.c_str());
  return res.size();
}

}  // namespace SoFCore
