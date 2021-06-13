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

#ifndef SOF_UTIL_SMALLVEC_INCLUDED
#define SOF_UTIL_SMALLVEC_INCLUDED

#include <algorithm>
#include <array>
#include <type_traits>

#include "util/misc.h"

namespace SoFUtil {

// Small vector with fixed storage. It may be useful in `constexpr` scope. The API of this class is
// the subset of `std::vector`'s API
template <typename T, size_t Size>
class SmallVector {
public:
  static_assert(std::is_trivial_v<T>);

  constexpr SmallVector() = default;

  constexpr bool empty() const { return size_ == 0; }
  constexpr size_t size() const { return size_; }

  constexpr T &operator[](const size_t idx) { return data_[idx]; }
  constexpr const T &operator[](const size_t idx) const { return data_[idx]; }

  using iterator = typename std::array<T, Size>::iterator;
  using const_iterator = typename std::array<T, Size>::const_iterator;

  constexpr iterator begin() { return data_.begin(); }
  constexpr iterator end() { return data_.begin() + size_; }
  constexpr const_iterator begin() const { return data_.begin(); }
  constexpr const_iterator end() const { return data_.begin() + size_; }

  constexpr void push_back(T item) {  // NOLINT(readability-identifier-naming)
    if (size_ == Size) {
      panic("SmallVector size exceeds its capacity");
    }
    data_[size_++] = std::move(item);
  }

  constexpr void pop_back() { --size_; }  // NOLINT(readability-identifier-naming)

  constexpr void clear() { size_ = 0; }

  constexpr bool operator==(const SmallVector &other) const {
    return std::equal(begin(), end(), other.begin(), other.end());
  }

  constexpr bool operator!=(const SmallVector &other) const { return !(*this == other); }

private:
  std::array<T, Size> data_{};
  size_t size_ = 0;
};

}  // namespace SoFUtil

#endif  // SOF_UTIL_SMALLVEC_INCLUDED
