// This file is part of SoFCheck
//
// Copyright (c) 2022 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_UTIL_COPY_PTR_INCLUDED
#define SOF_UTIL_COPY_PTR_INCLUDED

#include <memory>
#include <utility>

namespace SoFUtil {

// Like `std::unique_ptr<T>`, but has copy constructor and copy assignment operator, which copy an
// underlying object
template <typename T>
class CopyPtr {
public:
  CopyPtr() = default;
  explicit CopyPtr(T *value) : inner_(value) {}

  CopyPtr(CopyPtr &&) = default;
  CopyPtr &operator=(CopyPtr &&) = default;

  CopyPtr(const CopyPtr &other) : CopyPtr(other.inner_ ? new T(*other.inner_) : nullptr) {}
  CopyPtr &operator=(const CopyPtr &other) {
    if (this == &other) {
      return *this;
    }
    if (other.inner_) {
      if (inner_) {
        *inner_ = *other.inner_;
      } else {
        inner_ = std::unique_ptr<T>(new T(*other.inner_));
      }
    } else {
      inner_.release();
    }
    return *this;
  }

  ~CopyPtr() = default;

  T &operator*() { return *inner_; }
  const T &operator*() const { return *inner_; }

  T *operator->() { return inner_.get(); }
  const T *operator->() const { return inner_.get(); }

  T *get() const { return inner_.get(); }
  T *release() { return inner_.release(); }

private:
  std::unique_ptr<T> inner_;
};

template <typename T, typename... Args>
CopyPtr<T> makeCopyPtr(Args &&...args) {
  return CopyPtr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace SoFUtil

#endif  // SOF_UTIL_COPY_PTR_INCLUDED
