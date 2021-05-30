#ifndef SOF_UTIL_VALARRAY_INCLUDED
#define SOF_UTIL_VALARRAY_INCLUDED

#include <array>
#include <cstddef>

#include "util/operators.h"

namespace SoFUtil {

// This class behaves similar to `std::valarray<T>` in the standard library, but operates with the
// array of the fixed size. Optimizations with expression templates, which can be made in
// `std::valarray<T>` are sadly not available in this class.
template <typename T, size_t Size>
class ValArray {
public:
  ValArray() = default;

  explicit constexpr ValArray(std::array<T, Size> data) : data_(std::move(data)) {}

  // Returns the array filled with `T()`
  static constexpr ValArray zeroed() { return ValArray(std::array<T, Size>{}); }

  // Returns the size of the array;
  constexpr size_t size() { return data_.size(); }

  constexpr T &operator[](size_t idx) { return data_[idx]; }

  constexpr const T &operator[](size_t idx) const { return data_[idx]; }

  constexpr ValArray &operator+=(const ValArray &other) {
    for (size_t i = 0; i < Size; ++i) {
      data_[i] += other.data_[i];
    }
    return *this;
  }

  constexpr ValArray &operator-=(const ValArray &other) {
    for (size_t i = 0; i < Size; ++i) {
      data_[i] -= other.data_[i];
    }
    return *this;
  }

  constexpr ValArray &operator*=(const T &other) {
    for (size_t i = 0; i < Size; ++i) {
      data_[i] -= other;
    }
    return *this;
  }

  constexpr ValArray operator+() const { return *this; }

  constexpr ValArray operator-() const {
    ValArray result = *this;
    for (size_t i = 0; i < Size; ++i) {
      result.data_[i] = -result.data_[i];
    }
    return result;
  }

  SOF_PROPAGATE_CMP_OP(ValArray, data_, ==)
  SOF_PROPAGATE_CMP_OP(ValArray, data_, !=)
  SOF_VECTOR_OPS(ValArray, T)

private:
  std::array<T, Size> data_;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_VALARRAY_INCLUDED
