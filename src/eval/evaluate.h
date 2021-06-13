// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_EVAL_EVALUATE_INCLUDED
#define SOF_EVAL_EVALUATE_INCLUDED

#include <cstdint>
#include <utility>

#include "core/board.h"
#include "core/move.h"
#include "core/types.h"
#include "eval/types.h"

namespace SoFEval {

// Base class to perform position cost evaluation
template <typename S>
class Evaluator {
private:
  using Pair = typename ScoreTraits<S>::Pair;

public:
  class Tag {
  public:
    Tag() = default;

    // Returns the tag for the board `b`.
    static Tag from(const SoFCore::Board &b);

    // Returns the tag for the board which is obtained by applying move `move` to board `b`. Current
    // tag must be strictly equal to `Tag::from(b)`, i. e. `isValid(b)` must hold
    Tag updated(const SoFCore::Board &b, SoFCore::Move move) const;

    // Returns `true` if the tag is strictly equal to `Tag::from(b)`
    bool isValid(const SoFCore::Board &b) const {
      const Tag other = Tag::from(b);
      return inner_ == other.inner_ && stage_ == other.stage_;
    }

  private:
    explicit Tag(Pair inner, const uint32_t stage) : inner_(std::move(inner)), stage_(stage) {}

    Pair inner_;
    uint32_t stage_;

    template <typename>
    friend class Evaluator;
  };

  // Returns the position cost of `b`. `tag` must be strictly equal to `Tag::from(b)`, i. e.
  // `isValid(b)` must hold. The `evalForWhite` variant returns positive score if the position is
  // good for white, and negative score if the position is good for black. The `evaluateForCur`
  // variant returns positive score if the position is good for the moving side
  S evalForWhite(const SoFCore::Board &b, const Tag &tag);

  S evalForCur(const SoFCore::Board &b, const Tag &tag) {
    return evalForWhite(b, tag) * colorCoef(b.side);
  }

  // Returns a rough estimate of the position cost of `b`, based only on piece-square tables. `tag`
  // must be strictly equal to `Tag::from(b)`, i. e. `isValid(b)` must hold. To learn about the
  // difference between `evalMaterialForWhite()` and `evalMaterialForCur()`, see the `evalFor...()`
  // description
  inline S evalMaterialForWhite(const SoFCore::Board &, const Tag &tag) {
    return tag.inner_.first();
  }

  inline S evalMaterialForCur(const SoFCore::Board &b, const Tag &tag) {
    return evalMaterialForWhite(b, tag) * colorCoef(b.side);
  }

private:
  inline static constexpr int32_t colorCoef(const SoFCore::Color c) {
    return (c == SoFCore::Color::White) ? +1 : -1;
  }
};

}  // namespace SoFEval

#endif  // SOF_EVAL_EVALUATE_INCLUDED
