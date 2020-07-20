#include "util/strutil.h"

namespace SoFUtil {

const char *scanTokenEnd(const char *str) {
  while (*str != '\0' && *str != '\n' && *str != '\t' && *str != ' ') {
    ++str;
  }
  return str;
}

std::string sanitizeEol(std::string str) {
  for (char &c : str) {
    if (c < ' ' && c != '\t') {
      c = ' ';
    }
  }
  return str;
}

}  // namespace SoFUtil
