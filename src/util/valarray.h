// This file is part of SoFCheck
//
// Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_UTIL_VALARRAY_INCLUDED
#define SOF_UTIL_VALARRAY_INCLUDED

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "util/operators.h"

namespace SoFUtil {

// This class behaves similar to `std::valarray<T>` in the standard library, but operates with the
// array of the fixed size. Optimizations with expression templates, which can be made in
// `std::valarray<T>` are sadly not available in this class.
template <typename T, size_t Size>
class FixedValArray {
public:
  // Creates `FixedValArray` uninitialized
  FixedValArray() = default;

  explicit constexpr FixedValArray(std::array<T, Size> data) : data_(std::move(data)) {}

  // Returns the array filled with `T{}`
  static constexpr FixedValArray zeroed() { return FixedValArray(std::array<T, Size>{}); }

  // Returns the size of the array
  constexpr size_t size() const { return data_.size(); }

  constexpr T &operator[](const size_t idx) { return data_[idx]; }

  constexpr const T &operator[](const size_t idx) const { return data_[idx]; }

  constexpr FixedValArray &operator+=(const FixedValArray &other) {
    for (size_t i = 0; i < Size; ++i) {
      data_[i] += other.data_[i];
    }
    return *this;
  }

  constexpr FixedValArray &operator-=(const FixedValArray &other) {
    for (size_t i = 0; i < Size; ++i) {
      data_[i] -= other.data_[i];
    }
    return *this;
  }

  constexpr FixedValArray &operator*=(const T &other) {
    for (size_t i = 0; i < Size; ++i) {
      data_[i] *= other;
    }
    return *this;
  }

  constexpr FixedValArray operator+() const { return *this; }

  constexpr FixedValArray operator-() const {
    FixedValArray result = *this;
    for (size_t i = 0; i < Size; ++i) {
      result.data_[i] = -result.data_[i];
    }
    return result;
  }

  SOF_PROPAGATE_CMP_OP(FixedValArray, data_, ==)
  SOF_PROPAGATE_CMP_OP(FixedValArray, data_, !=)
  SOF_VECTOR_OPS(FixedValArray, T)

private:
  std::array<T, Size> data_;
};

// Index-value pair for `SparseValArray`
template <typename T>
struct IndexValuePair {
  size_t index;
  T value;
};

// This class is also similar `std::valarray<T>`, but is intended to be used with sparse data. The
// values are kept as an array of key-value pairs and the arrays are just concatenated during the
// arithmetical operations like `+=` or `-=`. `Storage` is the underlying type to store the pairs
template <typename T, typename Storage = std::vector<IndexValuePair<T>>>
class SparseValArray {
public:
  template <typename, typename>
  friend class SparseValArray;

  explicit constexpr SparseValArray(size_t size) : size_(size) {}

  constexpr SparseValArray(const SparseValArray &) = default;
  constexpr SparseValArray(SparseValArray &&) noexcept = default;
  constexpr SparseValArray &operator=(const SparseValArray &) = default;
  constexpr SparseValArray &operator=(SparseValArray &&) noexcept = default;

  template <typename Storage1>
  // NOLINTNEXTLINE(hicpp-explicit-conversions)
  constexpr SparseValArray(const SparseValArray<T, Storage1> &other) : size_(other.size_) {
    for (const auto &iter : other.storage_) {
      storage_.push_back(iter);
    }
  }

  template <typename Storage1>
  constexpr SparseValArray &operator=(const SparseValArray<T, Storage1> &other) {
    static_assert(!std::is_same_v<Storage, Storage1>,
                  "This is conversion operator, not copy operator");
    storage_.clear();
    for (const auto &iter : other.storage_) {
      storage_.push_back(iter);
    }
    size_ = other.size_;
    return *this;
  }

  constexpr size_t size() const { return size_; }

  // Increments position `idx` by `value`
  constexpr SparseValArray &add(const size_t idx, T value) {
    storage_.push_back(IndexValuePair<T>{idx, std::move(value)});
    tryCompactify();
    return *this;
  }

  // Returns the value of the array in its dense form
  std::vector<T> take() const {
    std::vector<T> result(size_);
    for (const auto &[key, value] : storage_) {
      result[key] += value;
    }
    return result;
  }

  // Tries to remove extra items from the array
  void compactify() {
    std::vector<T> vec = take();
    storage_.clear();
    for (size_t idx = 0; idx < size_; ++idx) {
      if (vec[idx] != T{}) {
        storage_.push_back(IndexValuePair<T>{idx, std::move(vec[idx])});
      }
    }
  }

  // `size()` == `other.size()` must hold, otherwise the behavior is undefined
  template <typename Storage1>
  constexpr SparseValArray &operator+=(const SparseValArray<T, Storage1> &other) {
    for (const auto &it : other.storage_) {
      storage_.push_back(it);
    }
    tryCompactify();
    return *this;
  }

  // `size()` == `other.size()` must hold, otherwise the behavior is undefined
  template <typename Storage1>
  constexpr SparseValArray &operator-=(const SparseValArray<T, Storage1> &other) {
    return *this += -other;
  }

  constexpr SparseValArray &operator*=(const T &other) {
    for (auto &it : storage_) {
      it.value *= other;
    }
    return *this;
  }

  constexpr SparseValArray &operator>>=(const T &other) {
    for (auto &it : storage_) {
      it.value >>= other;
    }
    return *this;
  }

  constexpr SparseValArray operator+() const { return *this; }

  constexpr SparseValArray operator-() const {
    SparseValArray result = *this;
    for (auto &it : result.storage_) {
      it.value = -it.value;
    }
    return result;
  }

  template <typename Storage1>
  constexpr bool operator==(const SparseValArray<T, Storage1> &other) const {
    return take() == other.take();
  }

  template <typename Storage1>
  constexpr bool operator!=(const SparseValArray<T, Storage1> &other) const {
    return take() != other.take();
  }

  SOF_VECTOR_OPS(SparseValArray, T)
  SOF_FROM_ASSIGNMENT_CLASS_OP(SparseValArray, T, >>)

private:
  constexpr void tryCompactify() {
    if (storage_.size() > 3 * size_) {
      compactify();
    }
  }

  Storage storage_;
  size_t size_;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_VALARRAY_INCLUDED
