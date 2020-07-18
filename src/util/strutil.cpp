#include "util/strutil.h"

namespace SoFUtil {

const char *scanTokenEnd(const char *str) {
  while (*str != '\0' && *str != '\n' && *str != '\t' && *str != ' ') {
    ++str;
  }
  return str;
}

}  // namespace SoFUtil
