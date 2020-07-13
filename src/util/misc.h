#ifndef SOF_MISC_COMPILER_INCLUDED
#define SOF_MISC_COMPILER_INCLUDED

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace SoFUtil {

[[noreturn]] void panic(const char *message);

}  // namespace SoFUtil

#endif  // SOF_MISC_COMPILER_INCLUDED
