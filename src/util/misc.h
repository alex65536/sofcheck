#ifndef SOF_MISC_COMPILER_INCLUDED
#define SOF_MISC_COMPILER_INCLUDED

// We define `likely` and `unlikely` ourselves, so reset them if they are present
#ifdef likely
#undef likely
#endif

#ifdef unlikely
#undef unlikely
#endif

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
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace SoFUtil {

// Terminates the program with the given `message`
[[noreturn]] void panic(const char *message);

}  // namespace SoFUtil

#endif  // SOF_MISC_COMPILER_INCLUDED
