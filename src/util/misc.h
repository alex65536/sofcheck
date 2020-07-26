#ifndef SOF_UTIL_MISC_INCLUDED
#define SOF_UTIL_MISC_INCLUDED

#include <string>

// Macros to help branch prediction
//
// If it's more likely that the condition is true, use `SOF_LIKELY`. Example:
//
// if (SOF_LIKELY(condition1)) {
//   doSomething();
// }
//
// The macro `SOF_UNLIKELY` is opposite to `SOF_LIKELY` and indicates that in most
// cases the given condition is false.
#define SOF_LIKELY(x) __builtin_expect(!!(x), 1)
#define SOF_UNLIKELY(x) __builtin_expect(!!(x), 0)

// Indicate that this part of code should never be reached. If the control flow reaches this
// statement, then the behaviour is undefined
#define SOF_UNREACHABLE() __builtin_unreachable()

// Indicate that the variable is unused and ignore compiler/linter warnings about it
#define SOF_UNUSED(x) (void)(x)

// Assume that x is true
#define SOF_ASSUME(x)          \
  {                            \
    if (!(x)) {                \
      __builtin_unreachable(); \
    }                          \
  }

namespace SoFUtil {

// Terminates the program with the given `message`
[[noreturn]] void panic(const char *message);
[[noreturn]] void panic(const std::string &message);

}  // namespace SoFUtil

#endif  // SOF_UTIL_MISC_INCLUDED
