#ifndef SOF_UTIL_STRUTIL_INCLUDED
#define SOF_UTIL_STRUTIL_INCLUDED

#include <cstdint>
#include <cstring>

namespace SoFUtil {

// Interperts the next token from the string `str` as unsigned integer and puts the result into
// `res`. Returns the number of characters read as unsigned integer. In case of error, -1 is
// returned instead.
int uintParse(uint32_t &res, const char *str);
int uintParse(uint16_t &res, const char *str);

// Writes the unsigned number `val` to the string `str`, including the terminating zero character.
// Returns the number of chars written, without counting the terminating zero.
int uintSave(uint32_t val, char *str);

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
