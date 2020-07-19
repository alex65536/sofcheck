#ifndef SOF_UTIL_MISC_INCLUDED
#define SOF_UTIL_MISC_INCLUDED

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
#undef likely
#endif
#define likely(x) __builtin_expect(!!(x), 1)

#ifdef unlikely
#undef unlikely
#endif
#define unlikely(x) __builtin_expect(!!(x), 0)

// Indicate that this part of code should never be reached. If the control flow reaches this
// statement, then the behaviour is undefined

#ifdef unreachable
#undef unreachable
#endif
#define unreachable() __builtin_unreachable()

// Indicate that the variable is unused and ignore compiler/linter warnings about it

#ifdef unused
#undef unused
#endif
#define unused(x) (void)(x)

namespace SoFUtil {

// Terminates the program with the given `message`
[[noreturn]] void panic(const char *message);

}  // namespace SoFUtil

#endif  // SOF_UTIL_MISC_INCLUDED
