#ifndef SOF_UTIL_MISC_INCLUDED
#define SOF_UTIL_MISC_INCLUDED

#include <string>

// Macros to help branch prediction
//
// If it's more likely that the condition is true, use `likely`. Example:
//
// if (likely(condition1)) {
//   doSomething();
// }
//
// The macro `unlikely` is opposite to `likely` and indicates that in most
// cases the given condition is false.

#ifdef likely
#error "likely" is already defined!
#endif
#define likely(x) __builtin_expect(!!(x), 1)

#ifdef unlikely
#error "unlikely" is already defined!
#endif
#define unlikely(x) __builtin_expect(!!(x), 0)

// Indicate that this part of code should never be reached. If the control flow reaches this
// statement, then the behaviour is undefined

#ifdef unreachable
#error "unreachable" is already defined!
#endif
#define unreachable() __builtin_unreachable()

// Indicate that the variable is unused and ignore compiler/linter warnings about it

#ifdef unused
#error "unused" is already defined!
#endif
#define unused(x) (void)(x)

// Assume that x is true

#ifdef assume
#error "assume" is already defined!
#endif
#define assume(x)              \
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
