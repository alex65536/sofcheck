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

#ifndef SOF_EVAL_COEFS_INCLUDED
#define SOF_EVAL_COEFS_INCLUDED

#include <cstdint>
#include <vector>

#include "eval/feature_count.h"
#include "eval/types.h"
#include "util/operators.h"
#include "util/smallvec.h"
#include "util/valarray.h"

namespace SoFEval {

// Underlying integer type for `BaseCoefs`
using coef_t = int32_t;

// Minimal unit in `BaseCoefs` that corresponds to one added coefficient. This constant is greater
// than `1` because we can perform the division to blend the ordinary score with the endgame score
constexpr coef_t COEF_UNIT = 256;

// Score type useful for tuning the feature weights. It doesn't keep the position cost, but keeps
// the number of times each feature is used instead. This is a base type, which has specializations
// with `std::vector<>` (for real work) and `SoFUtil::SmallVector<>` (for defining the weights in
// compile-time)
template <typename Storage>
class BaseCoefs {
public:
  template <typename>
  friend class BaseCoefs;

  constexpr BaseCoefs() : inner_(FEATURE_COUNT) {}

  template <typename Storage1>
  // NOLINTNEXTLINE(hicpp-explicit-conversions)
  constexpr BaseCoefs(const BaseCoefs<Storage1> &other) : inner_(other.inner_) {}

  template <typename Storage1>
  constexpr BaseCoefs &operator=(const BaseCoefs<Storage1> &other) {
    inner_ = other.inner_;
    return *this;
  }

  // Increments position `idx` by `value`
  constexpr BaseCoefs &add(const size_t idx, const coef_t value) {
    inner_.add(idx, value);
    return *this;
  }

  // Returns the coefficients as vector
  std::vector<coef_t> take() const { return inner_.take(); }

  SOF_PROPAGATE_VECTOR_OPS_NOCONST(BaseCoefs, coef_t, inner_)
  SOF_PROPAGATE_MUT_OP_EXT_NOCONST(BaseCoefs, inner_, >>=, coef_t)
  SOF_FROM_ASSIGNMENT_CLASS_OP_NOCONST(BaseCoefs, coef_t, >>)

private:
  using Inner = SoFUtil::SparseValArray<coef_t, Storage>;

  constexpr explicit BaseCoefs(Inner inner) : inner_(std::move(inner)) {}

  Inner inner_;
};

// Like `ScorePair`, but for `BaseCoefs`. This one also has specializations with `std::vector<>` and
// `SoFUtil::SmallVector<>`
template <typename Storage>
class BaseCoefsPair {
public:
  template <typename>
  friend class BaseCoefsPair;

  using Item = BaseCoefs<Storage>;

  constexpr BaseCoefsPair() = default;

  template <typename Storage1>
  // NOLINTNEXTLINE(hicpp-explicit-conversions)
  constexpr BaseCoefsPair(const BaseCoefsPair<Storage1> &other)
      : BaseCoefsPair({Item(other.first()), Item(other.second())}) {}

  template <typename Storage1>
  constexpr BaseCoefsPair &operator=(const BaseCoefsPair<Storage1> &other) {
    *this = BaseCoefsPair({Item(other.first()), Item(other.second())});
    return *this;
  }

  // Creates a score pair from two `Coefs`
  static constexpr BaseCoefsPair from(Item first, Item second) {
    return BaseCoefsPair({std::move(first), std::move(second)});
  }

  // Creates a score pair from two equal `Coefs`
  static constexpr BaseCoefsPair from(Item first) {
    Item second = first;
    return BaseCoefsPair({std::move(first), std::move(second)});
  }

  // Extracts first item from the pair
  constexpr const Item &first() const { return value_[0]; }

  // Extracts second item from the pair
  constexpr const Item &second() const { return value_[1]; }

  SOF_PROPAGATE_VECTOR_OPS(BaseCoefsPair, Item, value_)

private:
  explicit constexpr BaseCoefsPair(std::array<Item, 2> value) : value_(std::move(value)) {}
  explicit constexpr BaseCoefsPair(SoFUtil::FixedValArray<Item, 2> value)
      : value_(std::move(value)) {}

  SoFUtil::FixedValArray<Item, 2> value_;
};

// Definitions for `Coefs`
using Coefs = BaseCoefs<std::vector<SoFUtil::IndexValuePair<coef_t>>>;
using CoefsPair = BaseCoefsPair<std::vector<SoFUtil::IndexValuePair<coef_t>>>;

// Definitions for `SmallCoefs`
template <size_t Size>
using SmallCoefs = BaseCoefs<SoFUtil::SmallVector<SoFUtil::IndexValuePair<coef_t>, Size>>;

template <size_t Size>
using SmallCoefsPair = BaseCoefsPair<SoFUtil::SmallVector<SoFUtil::IndexValuePair<coef_t>, Size>>;

template <typename Storage>
struct ScoreTraits<BaseCoefs<Storage>> {
  using Pair = BaseCoefsPair<Storage>;

  // `BaseCoefs<>` cannot be compared, so `MIN` and `MAX` do not make any sense
  static constexpr SmallCoefs<1> MIN = SmallCoefs<1>();
  static constexpr SmallCoefs<1> MAX = SmallCoefs<1>();
};

}  // namespace SoFEval

#endif  // SOF_EVAL_COEFS_INCLUDED
