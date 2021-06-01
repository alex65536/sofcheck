#ifndef SOF_EVAL_EVALUATE_INCLUDED
#define SOF_EVAL_EVALUATE_INCLUDED

#include <cstdint>
#include <utility>

#include "core/board.h"
#include "core/move.h"
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
  // `isValid(b)` must hold
  S evaluate(const SoFCore::Board &b, const Tag tag);
};

}  // namespace SoFEval

#endif  // SOF_EVAL_EVALUATE_INCLUDED
