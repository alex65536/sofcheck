#ifndef SOF_UTIL_STRUTIL_INCLUDED
#define SOF_UTIL_STRUTIL_INCLUDED

#include <cstdint>

namespace SoFUtil {

// Gets the next token from str as uint32_t and puts the result into res
// Returns the number of chars read (or -1 in case of error)
int uintParse(uint32_t &res, const char *str);
int uintParse(uint16_t &res, const char *str);

// Writes res to the string str, including the terminating null character
// Returns the number of chars written
int uintSave(uint32_t val, char *str);

// Works like strcpy, but returns the address of null-terminating byte of dst
char *stpcpy(char *dst, const char *src);

}  // namespace SoFUtil

#endif  // SOF_UTIL_STRUTIL_INCLUDED
