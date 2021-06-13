// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

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

// Assume that x is true
#define SOF_ASSUME(x)          \
  {                            \
    if (!(x)) {                \
      __builtin_unreachable(); \
    }                          \
  }

// Concatenates two strings in preprocessor, even if one of the strings expands as a macro
#define SOF_PRIVATE_STRING_CONCAT(a, b) a##b
#define SOF_STRING_CONCAT(a, b) SOF_PRIVATE_STRING_CONCAT(a, b)

// Creates a unique name which starts with `prefix`
#define SOF_MAKE_UNIQUE(prefix) \
  SOF_STRING_CONCAT(SOF_STRING_CONCAT(prefix, __COUNTER__), SOF_STRING_CONCAT(l, __LINE__))

// Panics if the given condition is `false`. Unlike standard `assert`, the condition here is checked
// both in debug and release modes.
#define SOF_ASSERT(...)                                               \
  {                                                                   \
    if (SOF_UNLIKELY(!(__VA_ARGS__))) {                               \
      SoFUtil::Private::assertFail(__FILE__, __LINE__, #__VA_ARGS__); \
    }                                                                 \
  }

// Panics if the given condition is `false`. Unlike standard `assert`, the condition here is checked
// both in debug and release modes.
//
// This version displays `msg` instead of the expression in the error message.
#define SOF_ASSERT_MSG(msg, ...)                               \
  {                                                            \
    if (SOF_UNLIKELY(!(__VA_ARGS__))) {                        \
      SoFUtil::Private::assertFail(__FILE__, __LINE__, (msg)); \
    }                                                          \
  }

namespace SoFUtil {

// Terminates the program with the given `message`
[[noreturn]] void panic(const char *message);
[[noreturn]] void panic(const std::string &message);

namespace Private {

// Implementation detail of `SOF_ASSERT`
[[noreturn]] void assertFail(const char *file, int line, const char *msg);

}  // namespace Private

}  // namespace SoFUtil

#endif  // SOF_UTIL_MISC_INCLUDED
