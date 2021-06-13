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

#ifndef SOF_EVAL_PRIVATE_WEIGHT_TRAITS_INCLUDED
#define SOF_EVAL_PRIVATE_WEIGHT_TRAITS_INCLUDED

#include "eval/coefs.h"
#include "eval/private/weight_values.h"
#include "eval/score.h"
#include "eval/types.h"

namespace SoFEval::Private {

// Base struct that helps to declare weights for score type `T`. You must specify `Item` type for
// a simple item (e. g. a single coefficient) and `LargeItem` type for an item consisting of
// multiple coefficients (e. g. castling updates in piece-square table). Note that these types are
// not required to be equal to `T`, but they at least need to be implicitly convertible to `T`.
template <typename T>
struct WeightTraits {};

template <>
struct WeightTraits<score_t> {
  using Item = score_t;
  using LargeItem = score_t;

  static constexpr Item empty() { return 0; }
  static constexpr Item number(size_t num) { return WEIGHT_VALUES[num]; }
  static constexpr Item number(size_t n1, size_t n2) {
    return WEIGHT_VALUES[n1] + WEIGHT_VALUES[n2];
  }

  static constexpr Item negNumber(size_t num) { return -WEIGHT_VALUES[num]; }
  static constexpr Item negNumber(size_t n1, size_t n2) {
    return -(WEIGHT_VALUES[n1] + WEIGHT_VALUES[n2]);
  }
};

template <>
struct WeightTraits<Coefs> {
  using Item = SmallCoefs<2>;
  using LargeItem = SmallCoefs<8>;

  static constexpr Item empty() { return Item(); }

  static constexpr Item number(size_t num) { return Item().add(num, COEF_UNIT); }
  static constexpr Item number(size_t n1, size_t n2) {
    return Item().add(n1, COEF_UNIT).add(n2, COEF_UNIT);
  }

  static constexpr Item negNumber(size_t num) { return Item().add(num, -COEF_UNIT); }
  static constexpr Item negNumber(size_t n1, size_t n2) {
    return Item().add(n1, -COEF_UNIT).add(n2, -COEF_UNIT);
  }
};

}  // namespace SoFEval::Private

#endif  // SOF_EVAL_PRIVATE_WEIGHT_TRAITS_INCLUDED
