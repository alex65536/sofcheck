#ifndef SOF_UTIL_RESULT_INCLUDED
#define SOF_UTIL_RESULT_INCLUDED

#include <stdexcept>
#include <utility>
#include <variant>

#include "util/misc.h"

namespace SoFUtil {

// TODO: add more methods into `Result<T, E>`
// TODO: ensure that there are no unnecessary copies and moves in `Result<T, E>`

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
  inline T unwrap() noexcept {
    if (likely(isOk())) {
      return std::get<0>(variant_);
    }
    panic("Attempt to unwrap() Result<T, E> without a value");
  }

  // Consumes the `Result`, moving away `err()` value from it. If it doesn't hold the specified
  // value, it panics
  inline T unwrapErr() noexcept {
    if (likely(isErr())) {
      return std::get<1>(variant_);
    }
    panic("Attempt to unwrap() Result<T, E> without a value");
  }

  inline friend constexpr bool operator==(const std::variant<T, E> &r1,
                                          const std::variant<T, E> &r2) {
    return r1.variant_ == r2.variant_;
  }

  inline friend constexpr bool operator!=(const std::variant<T, E> &r1,
                                          const std::variant<T, E> &r2) {
    return r1.variant_ != r2.variant_;
  }

  Result(Ok<T> ok) : Result(std::in_place_index<0>, std::move(ok.value_)) {}
  Result(Err<E> err) : Result(std::in_place_index<1>, std::move(err.err_)) {}

private:
  inline constexpr explicit Result(std::in_place_index_t<0> idx, T &&value)
      : variant_(idx, std::forward<T>(value)) {}

  inline constexpr explicit Result(std::in_place_index_t<1> idx, E &&err)
      : variant_(idx, std::forward<E>(err)) {}

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
  inline explicit constexpr Ok(T value) : value_(std::move(value)) {}

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
  inline explicit constexpr Err(E err) : err_(std::move(err)) {}

private:
  E err_;

  template <typename, typename>
  friend class Result;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_RESULT_INCLUDED
