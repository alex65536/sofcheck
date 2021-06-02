#ifndef SOF_UTIL_STRUTIL_INCLUDED
#define SOF_UTIL_STRUTIL_INCLUDED

#include <charconv>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace SoFUtil {

// Scans the string forward, starting from `str` and returns the first character which is considered
// space
const char *scanTokenEnd(const char *str);

// Scans the string forward, starting from `str` and returns the first character which is not
// considered space
const char *scanTokenStart(const char *str);

// Returns `true` if `s` starts with `t`
bool startsWith(const std::string_view &s, const std::string_view &t);

// Splits zero-terminated string `str` into tokens
std::vector<std::string_view> split(const char *str);

// Converts the character `c` to lower case. `c` must be an ASCII character
inline constexpr char asciiToLower(const char c) {
  return ('A' <= c && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// Converts the character `c` to upper case. `c` must be an ASCII character
inline constexpr char asciiToUpper(const char c) {
  return ('a' <= c && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
}

// Returns `true` if the character is considered space
inline constexpr bool isSpace(const char ch) {
  return ch == '\n' || ch == '\t' || ch == '\r' || ch == ' ';
}

// Wrapper for `std::from_chars`. Tries to interpret the entire string between `first` and `last` as
// integer or floating point type `T`. Returns `true` and puts the result into `val` on success.
// Otherwise `false` is returned and `val` remains untouched.
template <typename T>
inline bool valueFromStr(const char *first, const char *last, T &val) {
  T value;
  const std::from_chars_result res = std::from_chars(first, last, value);
  if (res.ptr != last || res.ec != std::errc()) {
    return false;
  }
  val = value;
  return true;
}

// Same as `valueFromStr` above, but works with `std::string_view` instead of two `const char *`'s
template <typename T>
inline bool valueFromStr(const std::string_view &str, T &val) {
  return valueFromStr(str.data(), str.data() + str.size(), val);
}

// Replaces the small characters (with ASCII code <= 32) with spaces. The tab characters ('\t') are
// left intact. The primary purpose of this function is to sanitize the string for UCI output, to
// make sure that it doesn't contain a newline, which can be potentially interpreted as a new UCI
// command
std::string sanitizeEol(std::string str);

// Removes leading characters satisfying the predicate from the string
template <typename Pred>
inline constexpr const char *trimLeft(const char *str, Pred pred) {
  while (pred(*str)) {
    ++str;
  }
  return str;
}

// Removes leading spaces from the string
inline constexpr const char *trimLeft(const char *str) { return trimLeft(str, isSpace); }

// Removes leading line endings from the string
inline constexpr const char *trimEolLeft(const char *str) {
  return trimLeft(str, [](const char c) { return c == '\n' || c == '\r'; });
}

// Removes leading and trailing spaces from the string
std::string_view trim(const std::string_view &str);

// Converts the string `s` to lower case. `s` must be a string consisting of ASCII characters
void asciiToLower(std::string &s);

// Converts the string `s` to upper case. `s` must be a string consisting of ASCII characters
void asciiToUpper(std::string &s);

// Replaces all the entries of character `src` to the character `dst` in the string `s`
void replace(std::string &s, char src, char dst);

// Works like `strcpy`, but returns the address of null-terminating byte in `dst` (e.g. pointer to
// the end of `dst` instead of the pointer to the beginning of `dst`)
//
// For more details, see https://www.man7.org/linux/man-pages/man3/stpcpy.3.html
#ifdef USE_SYSTEM_STPCPY
inline char *stpcpy(char *dst, const char *src) { return ::stpcpy(dst, src); }
#else
inline char *stpcpy(char *dst, const char *src) {
  strcpy(dst, src);  // NOLINT
  return dst + strlen(src);
}
#endif

}  // namespace SoFUtil

#endif  // SOF_UTIL_STRUTIL_INCLUDED
