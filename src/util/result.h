// This file is part of SoFCheck
//
// Copyright (c) 2020-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_UTIL_RESULT_INCLUDED
#define SOF_UTIL_RESULT_INCLUDED

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "util/misc.h"

namespace SoFUtil {

// TODO : add more methods into `Result<T, E>`
// TODO : ensure that there are no unnecessary copies and moves in `Result<T, E>`

template <typename T>
class Ok;

template <typename E>
class Err;

// A simple result type that behaves similar to `Result<T, E>` in Rust
template <typename T, typename E>
class Result {
public:
  inline constexpr bool isOk() const { return variant_.index() == 0; }

  inline constexpr bool isErr() const { return variant_.index() == 1; }

  // Consumes the Result, moving away `ok()` value from it. If it doesn't hold the specified value,
  // it panics
  inline constexpr T unwrap() && /**/ noexcept {
    if (SOF_LIKELY(isOk())) {
      return std::move(std::get<0>(variant_));
    }
    panic("Attempt to unwrap() Result<T, E> without a value");
  }

  // Transforms the `ok()` value using function `f`, otherwise propagates the error
  template <typename F>
  auto map(F f) && /**/ noexcept
      -> Result<std::decay_t<decltype(f(std::move(std::declval<T>())))>, E> {
    if (isOk()) {
      return Ok(f(std::move(std::get<0>(variant_))));
    }
    return Err(std::move(std::get<1>(variant_)));
  }

  // Transforms the `err()` value using function `f`, otherwise propagates the result
  template <typename F>
  auto mapErr(F f) && /**/ noexcept
      -> Result<T, std::decay_t<decltype(f(std::move(std::declval<E>())))>> {
    if (isErr()) {
      return Err(f(std::move(std::get<1>(variant_))));
    }
    return Ok(std::move(std::get<0>(variant_)));
  }

  // Consumes the Result, moving away `ok()` value from it. If it doesn't hold the specified value,
  // it gets the result from `f`
  template <typename F>
  inline constexpr T okOr(F f) && /**/ noexcept {
    if (isOk()) {
      return std::move(std::get<0>(variant_));
    }
    return f();
  }

  // Consumes the Result, moving away `ok()` value from it. If it doesn't hold the specified value,
  // it calls `handler`, which should be noreturn. If `handler` returns, the function just panics
  template <typename Handler>
  inline constexpr T okOrErr(Handler handler) && /**/ noexcept {
    if (SOF_LIKELY(isOk())) {
      return std::move(std::get<0>(variant_));
    }
    handler(std::move(std::get<1>(variant_)));
    panic("Attempt to unwrap() Result<T, E> without a value");
  }

  // Consumes the `Result`, moving away `err()` value from it. If it doesn't hold the specified
  // value, it panics
  inline constexpr E unwrapErr() && /**/ noexcept {
    if (SOF_LIKELY(isErr())) {
      return std::move(std::get<1>(variant_));
    }
    panic("Attempt to unwrapErr() Result<T, E> without an error");
  }

  // Returns a view on `ok()` value. If it doesn't hold the specified value, it panics
  inline constexpr const T &ok() const noexcept {
    if (SOF_LIKELY(isOk())) {
      return std::get<0>(variant_);
    }
    panic("Attempt to ok() Result<T, E> without a value");
  }

  // Returns a view on `ok()` value. If it doesn't hold the specified value, it panics
  inline constexpr T &ok() noexcept {
    if (SOF_LIKELY(isOk())) {
      return std::get<0>(variant_);
    }
    panic("Attempt to ok() Result<T, E> without a value");
  }

  // Returns a view on `err()` value. If it doesn't hold the specified value, it panics
  inline constexpr const E &err() const noexcept {
    if (SOF_LIKELY(isErr())) {
      return std::get<1>(variant_);
    }
    panic("Attempt to err() Result<T, E> without an error");
  }

  inline constexpr E &err() noexcept {
    if (SOF_LIKELY(isErr())) {
      return std::get<1>(variant_);
    }
    panic("Attempt to err() Result<T, E> without an error");
  }

  inline friend constexpr bool operator==(const Result<T, E> &r1, const Result<T, E> &r2) {
    return r1.variant_ == r2.variant_;
  }

  inline friend constexpr bool operator!=(const Result<T, E> &r1, const Result<T, E> &r2) {
    return r1.variant_ != r2.variant_;
  }

  // NOLINTNEXTLINE(hicpp-explicit-conversions)
  inline constexpr Result(Ok<T> &&ok) noexcept
      : variant_(std::in_place_index<0>, std::move(ok.value_)) {}

  // NOLINTNEXTLINE(hicpp-explicit-conversions)
  inline constexpr Result(Err<E> &&err) noexcept
      : variant_(std::in_place_index<1>, std::move(err.err_)) {}

private:
  std::variant<T, E> variant_;
};

// Wrapper class that helps to convert the value into successful `Result<T, E>`
//
// Usage example:
//
// Result<int, std::string> res = Ok(42);
template <typename T>
class Ok {
public:
  inline constexpr explicit Ok(const T &value) : value_(value) {}
  inline constexpr explicit Ok(T &&value) noexcept : value_(std::move(value)) {}

private:
  T value_;

  template <typename, typename>
  friend class Result;
};

// Wrapper class that helps to convert the value into errored `Result<T, E>`
//
// Usage example:
//
// Result<int, std::string> res = Err("something is wrong");
template <typename E>
class Err {
public:
  inline constexpr explicit Err(const E &err) : err_(err) {}
  inline constexpr explicit Err(E &&err) noexcept : err_(std::move(err)) {}

private:
  E err_;

  template <typename, typename>
  friend class Result;
};

#define SOF_PRIVATE_TRY_DECL_IMPL(var, tempVar, expr)                    \
  auto tempVar = (expr); /* NOLINT */                                    \
  if (tempVar.isErr()) { /* NOLINT */                                    \
    return SoFUtil::Err(std::move(tempVar).unwrapErr());                 \
  }                                                                      \
  auto var = std::move(tempVar).unwrap(); /* NOLINT */                   \
  do {                                                                   \
    /* Do nothing; this statement is added just to force `;` after it */ \
  } while (false)

#define SOF_PRIVATE_TRY_ASSIGN_IMPL(var, tempVar, expr)    \
  do {                                                     \
    auto tempVar = (expr); /* NOLINT */                    \
    if (tempVar.isErr()) { /* NOLINT */                    \
      return SoFUtil::Err(std::move(tempVar).unwrapErr()); \
    }                                                      \
    (var) = std::move(tempVar).unwrap();                   \
  } while (false)

#define SOF_PRIVATE_TRY_CONSUME_IMPL(tempVar, expr)                                       \
  do {                                                                                    \
    auto tempVar = (expr); /* NOLINT */                                                   \
    if (tempVar.isErr()) { /* NOLINT */                                                   \
      return SoFUtil::Err(std::move(tempVar).unwrapErr());                                \
    }                                                                                     \
    static_assert(std::is_same_v<decltype(std::move(tempVar).unwrap()), std::monostate>); \
  } while (false)

// Runs `expr`, which is some expression returning `Result<T, E>`. If the obtained result is
// successful, then a new variable `var` is created with this result. Otherwise, propagates the
// error as the return value. Be careful with this macro, as it consists of multiple statements
// without enclosing braces
#define SOF_TRY_DECL(var, expr) \
  SOF_PRIVATE_TRY_DECL_IMPL(var, SOF_MAKE_UNIQUE(sofResultTryDecl__), expr)

// Runs `expr`, which is some expression returning `Result<T, E>`. If the obtained result is
// successful, then it's assigned to the variable `var`. Otherwise, propagates the error as the
// return value
#define SOF_TRY_ASSIGN(var, expr) \
  SOF_PRIVATE_TRY_ASSIGN_IMPL(var, SOF_MAKE_UNIQUE(sofResultTryAssign__), expr)

// Runs `expr`, which is some expression returning `Result<std::monostate, E>`. If the obtained
// result is unsuccessful, propagates the error as the return value
#define SOF_TRY_CONSUME(expr) \
  SOF_PRIVATE_TRY_CONSUME_IMPL(SOF_MAKE_UNIQUE(sofResultTryConsume__), expr)

}  // namespace SoFUtil

#endif  // SOF_UTIL_RESULT_INCLUDED
