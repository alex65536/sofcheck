#ifndef SOF_EVAL_COEFS_INCLUDED
#define SOF_EVAL_COEFS_INCLUDED

#include <cstdint>

#include "eval/feature_count.h"
#include "util/operators.h"
#include "util/valarray.h"

namespace SoFEval {

// Underlying integer type for `Coefs`
using coef_t = int16_t;

// Score type useful for tuning the feature weights. It doesn't keep the position cost, but keeps
// the number of times each feature is used instead.
using Coefs = SoFUtil::ValArray<coef_t, FEATURE_COUNT>;

// Create `Coefs` with a single value `val` at the position `pos`
inline static constexpr Coefs makeCoefs(coef_t val, size_t pos) {
  auto result = Coefs::zeroed();
  result[pos] = val;
  return result;
}

// Create `Coefs` with a two ones at the position `pos1` and at position `pos2`
inline static constexpr Coefs makeCoefs(coef_t val, size_t pos1, size_t pos2) {
  auto result = Coefs::zeroed();
  result[pos1] += val;
  result[pos2] += val;
  return result;
}

// Like `ScorePair`, but for `Coefs`
class CoefsPair {
public:
  CoefsPair() = default;

  // Creates a score pair from two `Coefs`
  static constexpr CoefsPair from(Coefs first, Coefs second) { return CoefsPair({first, second}); }

  // Creates a score pair from two equal `Coefs`
  static constexpr CoefsPair from(Coefs first) { return CoefsPair({first, first}); }

  // Extracts first item from the pair
  constexpr const Coefs &first() const { return value_[0]; }

  // Extracts second item from the pair
  constexpr const Coefs &second() const { return value_[1]; }

  SOF_PROPAGATE_VECTOR_OPS(CoefsPair, Coefs, value_)

private:
  explicit constexpr CoefsPair(std::array<Coefs, 2> value) : value_(value) {}
  explicit constexpr CoefsPair(SoFUtil::ValArray<Coefs, 2> value) : value_(value) {}

  SoFUtil::ValArray<Coefs, 2> value_;
};

template <>
struct ScoreTraits<Coefs> {
  using Pair = CoefsPair;

  // `Coefs` cannot be compared, so `MIN` and `MAX` do not make any sense
  static constexpr Coefs MIN = Coefs::zeroed();
  static constexpr Coefs MAX = Coefs::zeroed();
};

}  // namespace SoFEval

#endif  // SOF_EVAL_COEFS_INCLUDED
