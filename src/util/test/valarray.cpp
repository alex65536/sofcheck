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

#include "util/valarray.h"

#include <gtest/gtest.h>

#include <random>

#include "util/smallvec.h"

using SoFUtil::FixedValArray;
using SoFUtil::SparseValArray;

TEST(SoFUtil, FixedValArray_Size) {
  auto arr = FixedValArray<int, 10>::zeroed();
  ASSERT_EQ(arr.size(), 10);
}

TEST(SoFUtil, FixedValArray_Values) {
  FixedValArray<int, 6> arr({1, 4, 2, 8, 5, 7});
  EXPECT_EQ(arr[0], 1);
  EXPECT_EQ(arr[1], 4);
  EXPECT_EQ(arr[2], 2);
  EXPECT_EQ(arr[3], 8);
  EXPECT_EQ(arr[4], 5);
  EXPECT_EQ(arr[5], 7);
}

TEST(SoFUtil, FixedValArray_Operators) {
  using Arr = FixedValArray<int, 5>;

  const Arr first({1, 2, 3, 4, 5});
  const Arr second({4, 2, 1, 3, 10});
  const Arr sum({5, 4, 4, 7, 15});
  const Arr dif({-3, 0, 2, 1, -5});
  const Arr mul({3, 6, 9, 12, 15});
  const Arr neg({-4, -2, -1, -3, -10});

  EXPECT_EQ(first + second, sum);
  EXPECT_EQ(first - second, dif);
  EXPECT_EQ(first * 3, mul);
  EXPECT_EQ(3 * first, mul);
  EXPECT_EQ(-second, neg);
}

template <typename T>
inline static std::vector<T> toVec(T a1, T a2, T a3, T a4, T a5, T a6) {
  return std::vector<T>{std::move(a1), std::move(a2), std::move(a3),
                        std::move(a4), std::move(a5), std::move(a6)};
}

template <typename T>
inline static std::vector<T> toVec(T a1, T a2, T a3, T a4, T a5) {
  return std::vector<T>{std::move(a1), std::move(a2), std::move(a3), std::move(a4), std::move(a5)};
}

TEST(SoFUtil, SparseValArray_Base) {
  using Arr = SparseValArray<int>;
  using SmallArr = SparseValArray<int, SoFUtil::SmallVector<SoFUtil::IndexValuePair<int>, 6>>;

  Arr arr(6);
  EXPECT_EQ(arr.take(), toVec(0, 0, 0, 0, 0, 0));
  EXPECT_EQ(arr.size(), 6);
  arr.add(3, 4).add(2, 5).add(3, -1);
  EXPECT_EQ(arr.take(), toVec(0, 0, 5, 3, 0, 0));

  arr = SmallArr(6).add(1, 2).add(0, 5).add(1, -3).add(2, 4).add(1, 42);
  EXPECT_EQ(arr.take(), toVec(5, 41, 4, 0, 0, 0));

  arr = SmallArr(5).add(1, 2).add(0, 5).add(1, -3).add(2, 4).add(1, 42);
  EXPECT_EQ(arr.take(), toVec(5, 41, 4, 0, 0));

  Arr arr2 = SmallArr(6).add(1, 2).add(0, 5).add(1, -3).add(2, 4).add(1, 42).add(5, 3);
  EXPECT_EQ(arr2.take(), toVec(5, 41, 4, 0, 0, 3));

  Arr arr3 = SmallArr(5).add(1, 2).add(0, 5).add(1, -3).add(2, 4).add(1, 42);
  EXPECT_EQ(arr3.take(), toVec(5, 41, 4, 0, 0));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
  arr = Arr(6).add(1, 2).add(0, 5).add(1, -3).add(2, 4).add(1, 42);
  arr = arr;  // NOLINT
  EXPECT_EQ(arr.take(), toVec(5, 41, 4, 0, 0, 0));
#pragma GCC diagnostic pop
}

TEST(SoFUtil, SparseValArray_Compactify) {
  using Arr = SparseValArray<int>;

  std::mt19937 rnd(42);  // NOLINT(cert-msc32-c, cert-msc51-cpp): use fixed seed
  Arr arr(5);
  std::vector<int> v(5);
  for (size_t attempt = 0; attempt < 1'000; ++attempt) {
    const size_t pos = rnd() % 5;
    const int val = static_cast<int>(rnd() % 10);
    v[pos] += val;
    arr.add(pos, val);
  }
  EXPECT_EQ(arr.take(), v);

  Arr arr2(6);
  for (size_t idx = 0; idx < 12; ++idx) {
    arr2.add(4, 1);
  }
  EXPECT_EQ(arr2.take(), toVec(0, 0, 0, 0, 12, 0));
  arr2.compactify();
  EXPECT_EQ(arr2.take(), toVec(0, 0, 0, 0, 12, 0));
}

TEST(SoFUtil, SparseValArray_Equality) {
  using Arr = SparseValArray<int>;
  using SmallArr = SparseValArray<int, SoFUtil::SmallVector<SoFUtil::IndexValuePair<int>, 6>>;

  const auto first = SmallArr(6).add(1, 5).add(5, 4).add(1, -1).add(2, 3);
  const auto second = SmallArr(6).add(2, 3).add(5, 4).add(1, 4).add(3, 0);
  const auto third = SmallArr(6).add(2, 3).add(5, 4).add(3, 2);

  const Arr largeFirst = first;
  const Arr largeSecond = second;
  const Arr largeThird = third;

  EXPECT_TRUE(first == second);
  EXPECT_TRUE(first != third);
  EXPECT_TRUE(largeFirst == largeSecond);
  EXPECT_TRUE(largeFirst != largeThird);

  EXPECT_TRUE(first == largeFirst);
  EXPECT_TRUE(largeSecond == second);
  EXPECT_TRUE(third == largeThird);

  EXPECT_TRUE(largeSecond == first);
  EXPECT_TRUE(second == largeFirst);
  EXPECT_TRUE(largeFirst != third);
  EXPECT_TRUE(largeThird != first);
}

TEST(SoFUtil, SparseValArray_Operators) {
  using Arr = SparseValArray<int>;
  using SmallArr = SparseValArray<int, SoFUtil::SmallVector<SoFUtil::IndexValuePair<int>, 6>>;

  {
    const auto arr = Arr(5).add(3, 4).add(2, -1).add(1, 2).add(3, 8);
    EXPECT_EQ((+arr).take(), toVec(0, 2, -1, 12, 0));
    EXPECT_EQ((-arr).take(), toVec(0, -2, 1, -12, 0));
  }

  {
    auto arr = Arr(5).add(3, 4).add(2, -1).add(1, 2).add(3, 8);
    arr += Arr(5).add(2, 8).add(1, -3).add(4, 5);
    EXPECT_EQ(arr.take(), toVec(0, -1, 7, 12, 5));
    arr += SmallArr(5).add(3, 1).add(0, -2).add(4, -5).add(3, -2);
    EXPECT_EQ(arr.take(), toVec(-2, -1, 7, 11, 0));
  }

  {
    auto arr = Arr(5).add(3, 4).add(2, -1).add(1, 2).add(3, 8);
    arr -= Arr(5).add(2, 8).add(1, -3).add(4, 5);
    EXPECT_EQ(arr.take(), toVec(0, 5, -9, 12, -5));
    arr -= SmallArr(5).add(3, 1).add(0, -2).add(4, -5).add(3, -2);
    EXPECT_EQ(arr.take(), toVec(2, 5, -9, 13, 0));
  }

  {
    auto arr = Arr(5).add(3, 4).add(2, -1).add(1, 2).add(3, 8);
    arr *= 2;
    EXPECT_EQ(arr.take(), toVec(0, 4, -2, 24, 0));
  }

  const auto first = Arr(5).add(3, 4).add(2, -1).add(1, 2).add(3, 8);
  const auto second = Arr(5).add(2, 8).add(1, -3).add(4, 5);
  EXPECT_EQ((first + second).take(), toVec(0, -1, 7, 12, 5));
  EXPECT_EQ((first - second).take(), toVec(0, 5, -9, 12, -5));
  EXPECT_EQ((first * 2).take(), toVec(0, 4, -2, 24, 0));
  EXPECT_EQ((2 * first).take(), toVec(0, 4, -2, 24, 0));
}
