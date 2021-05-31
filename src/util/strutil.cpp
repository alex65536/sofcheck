#include "util/strutil.h"

namespace SoFUtil {

const char *scanTokenEnd(const char *str) {
  while (*str != '\0' && !isSpace(*str)) {
    ++str;
  }
  return str;
}

const char *scanTokenStart(const char *str) {
  while (isSpace(*str)) {
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

std::string trim(const std::string &str) {
  size_t left = 0;
  size_t right = str.size();
  while (left < str.size() && isSpace(str[left])) {
    ++left;
  }
  if (left == right) {
    return std::string();
  }
  while (right != 0 && isSpace(str[right - 1])) {
    --right;
  }
  return str.substr(left, right - left);
}

void asciiToLower(std::string &s) {
  for (char &ch : s) {
    ch = asciiToLower(ch);
  }
}

void asciiToUpper(std::string &s) {
  for (char &ch : s) {
    ch = asciiToUpper(ch);
  }
}

void replace(std::string &s, const char src, const char dst) {
  for (char &ch : s) {
    ch = (ch == src) ? dst : ch;
  }
}

}  // namespace SoFUtil
