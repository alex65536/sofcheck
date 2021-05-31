#ifndef SOF_UTIL_VALARRAY_INCLUDED
#define SOF_UTIL_VALARRAY_INCLUDED

#include <array>
#include <cstddef>
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

  // Returns the array filled with `T()`
  static constexpr FixedValArray zeroed() { return FixedValArray(std::array<T, Size>{}); }

  // Returns the size of the array;
  constexpr size_t size() { return data_.size(); }

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

// This class is also similar `std::valarray`, but is intended to be used with sparse data. The
// values are kept as an array of key-value pairs and are just concatenated during the arithmetical
// operations.
template <typename T, typename Storage = std::vector<IndexValuePair<T>>>
class SparseValArray {
public:
  template <typename, typename>
  friend class SparseValArray;

  explicit constexpr SparseValArray(size_t size) : size_(size) {}

  template <typename Storage1>
  // NOLINTNEXTLINE(hicpp-explicit-conversions)
  constexpr SparseValArray(const SparseValArray<T, Storage1> &other) : size_(other.size_) {
    for (const auto &iter : other.storage_) {
      storage_.push_back(iter);
    }
  }

  template <typename Storage1>
  constexpr SparseValArray &operator=(const SparseValArray<T, Storage1> &other) {
    Storage newStorage;
    for (const auto &iter : other.storage_) {
      newStorage.push_back(iter);
    }
    storage_ = std::move(newStorage);
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

  // Try to remove extra items from the array
  void compactify() {
    std::vector<T> vec = take();
    storage_.clear();
    for (size_t idx = 0; idx < size_; ++idx) {
      if (vec[idx] != T{}) {
        storage_.push_back(IndexValuePair<T>{idx, std::move(vec[idx])});
      }
    }
  }

  // `size()` == `other.size()` must hold, otherwise the behaviour is undefined
  template <typename Storage1>
  constexpr SparseValArray &operator+=(const SparseValArray<T, Storage1> &other) {
    for (const auto &it : other.storage_) {
      storage_.push_back(it);
    }
    tryCompactify();
    return *this;
  }

  // `size()` == `other.size()` must hold, otherwise the behaviour is undefined
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
