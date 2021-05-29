#ifndef SOF_EVAL_EVALUATE_INCLUDED
#define SOF_EVAL_EVALUATE_INCLUDED

#include "core/board.h"
#include "core/move.h"
#include "eval/score.h"

namespace SoFEval {

// Returns the position cost of `b`. `psq` must be strictly equal to `boardGetPsqScore(b)`.
inline score_t evaluate([[maybe_unused]] const SoFCore::Board &b, const ScorePair psq) {
  return psq.first();
}

// Returns the position cost of `b` based on piece-square tables.
ScorePair boardGetPsqScore(const SoFCore::Board &b);

// Returns the position cost of the board which is obtained by applying move `move` to board `b`.
// This position cost is based on piece-square tables. `psq` must be strictly equal to
// `boardGetPsqScore(b)`.
ScorePair boardUpdatePsqScore(const SoFCore::Board &b, SoFCore::Move move, ScorePair psq);

}  // namespace SoFEval

#endif  // SOF_EVAL_EVALUATE_INCLUDED
