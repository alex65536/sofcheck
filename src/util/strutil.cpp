#include "util/strutil.h"

#include <algorithm>

#include "util/math.h"

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

std::vector<std::string_view> split(const char *str) {
  std::vector<std::string_view> result;
  str = scanTokenStart(str);
  while (*str != '\0') {
    const char *next = scanTokenEnd(str);
    result.emplace_back(str, next - str);
    str = scanTokenStart(next);
  }
  return result;
}

bool startsWith(const std::string_view &s, const std::string_view &t) {
  return s.size() >= t.size() && s.substr(0, t.size()) == t;
}

std::string sanitizeEol(std::string str) {
  for (char &c : str) {
    if (c < ' ' && c != '\t') {
      c = ' ';
    }
  }
  return str;
}

std::string_view trim(const std::string_view &str) {
  size_t left = 0;
  size_t right = str.size();
  while (left < str.size() && isSpace(str[left])) {
    ++left;
  }
  if (left == right) {
    return std::string_view();
  }
  while (right != 0 && isSpace(str[right - 1])) {
    --right;
  }
  return str.substr(left, right - left);
}

std::string trimmed(const std::string &str) {
  std::string result(trim(str));
  return result;
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

size_t intStrLen(const int64_t value) {
  return uintStrLen(std::abs(value)) + ((value < 0) ? 1 : 0);
}

size_t uintStrLen(const uint64_t value) { return (value == 0) ? 1 : 1 + SoFUtil::log10(value); }

}  // namespace SoFUtil
