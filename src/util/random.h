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

#ifndef SOF_UTIL_RANDOM_INCLUDED
#define SOF_UTIL_RANDOM_INCLUDED

#include <algorithm>
#include <cstdint>
#include <limits>

namespace SoFUtil {

// Generates a random number.
//
// It is disregarded to use this function during static initialization.
uint64_t random();

template <typename Iter>
void randomShuffle(Iter first, Iter last) {
  struct Random {
    using result_type = uint64_t;
    inline result_type operator()() { return random(); }
    inline static constexpr result_type min() { return std::numeric_limits<result_type>::min(); }
    inline static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
  };
  std::shuffle(first, last, Random{});
}

}  // namespace SoFUtil

#endif  // SOF_UTIL_RANDOM_INCLUDED
