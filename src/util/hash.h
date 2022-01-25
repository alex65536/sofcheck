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

#ifndef SOF_UTIL_HASH_INCLUDED
#define SOF_UTIL_HASH_INCLUDED

#include "util/bit.h"

namespace SoFUtil {

// The code here derives from FarmHash64 by Google:
// https://github.com/google/farmhash/blob/master/src/farmhash.cc.
//
// FarmHash is distributed under the following license terms:
//
// Copyright (c) 2014 Google, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

// Compute hash of a pair of two 8-byte numbers, `v0` and `v1`
inline constexpr uint64_t hash16(uint64_t v0, uint64_t v1);

// Compute hash of a tuple of four 8-byte numbers, `v0`, `v1`, `v2` and `v3`
inline constexpr uint64_t hash32(uint64_t v0, uint64_t v1, uint64_t v2, uint64_t v3);

namespace Private {

// Some primes between 2^63 and 2^64 for various uses.
constexpr uint64_t HASH_K1 = 0xb492b66fbe98f273ULL;
constexpr uint64_t HASH_K2 = 0x9ae16a3b2f90404fULL;

// Helper function, originally called `HashLen16` in FarmHash
inline constexpr uint64_t hashFinalize(const uint64_t u, const uint64_t v, const uint64_t mul) {
  // Murmur-inspired hashing.
  uint64_t a = (u ^ v) * mul;
  a ^= (a >> 47);
  uint64_t b = (v ^ a) * mul;
  b ^= (b >> 47);
  b *= mul;
  return b;
}

}  // namespace Private

inline constexpr uint64_t hash16(const uint64_t v0, const uint64_t v1) {
  constexpr uint64_t len = 16;
  const uint64_t mul = Private::HASH_K2 + len * 2;
  const uint64_t a = v0 + Private::HASH_K2;
  const uint64_t b = v1;
  const uint64_t c = rotateRight(b, 37) * mul + a;
  const uint64_t d = (rotateRight(a, 25) + b) * mul;
  return Private::hashFinalize(c, d, mul);
}

inline constexpr uint64_t hash32(const uint64_t v0, const uint64_t v1, const uint64_t v2,
                                 const uint64_t v3) {
  constexpr uint64_t len = 32;
  const uint64_t mul = Private::HASH_K2 + len * 2;
  const uint64_t a = v0 * Private::HASH_K1;
  const uint64_t b = v1;
  const uint64_t c = v2 * mul;
  const uint64_t d = v3 * Private::HASH_K2;
  return Private::hashFinalize(rotateRight(a + b, 43) + rotateRight(c, 30) + d,
                               a + rotateRight(b + Private::HASH_K2, 18) + c, mul);
}

}  // namespace SoFUtil

#endif  // SOF_UTIL_HASH_INCLUDED
