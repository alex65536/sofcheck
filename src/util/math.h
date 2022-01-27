// This file is part of SoFCheck
//
// Copyright (c) 2021-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_UTIL_MATH_INCLUDED
#define SOF_UTIL_MATH_INCLUDED

#include <cstddef>
#include <cstdint>

namespace SoFUtil {

// Computes `floor(log10(val))` where `val` is a positive number
inline constexpr size_t log10(uint64_t val) {
  // Stub to prevent integer overflow
  if (val >= static_cast<uint64_t>(1e19)) {
    return 19;
  }

  size_t res = 0;
  uint64_t pow = 1;
  while (pow <= val) {
    pow *= 10;
    ++res;
  }
  return res - 1;
}

// Returns `std::abs(x - y)`. Unlike the former, it is safe to use for both signed and unsigned
// integers
template <typename Int>
inline constexpr Int absDiff(const Int x, const Int y) {
  return x > y ? x - y : y - x;
}

}  // namespace SoFUtil

#endif  // SOF_UTIL_MATH_INCLUDED
