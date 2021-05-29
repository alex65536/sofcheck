#ifndef SOF_EVAL_EVALUATE_INCLUDED
#define SOF_EVAL_EVALUATE_INCLUDED

#include "core/board.h"
#include "core/move.h"
#include "eval/score.h"

namespace SoFEval {

// Returns the position cost of `b`. `psq` must be strictly equal to `boardGetPsqScore(b)`.
inline score_pair_t evaluate([[maybe_unused]] const SoFCore::Board &b, const score_pair_t psq) {
  return scorePairFirst(psq);
}

// Returns the position cost of `b` based on piece-square tables.
score_pair_t boardGetPsqScore(const SoFCore::Board &b);

// Returns the position cost of the board which is obtained by applying move `move` to board `b`.
// This position cost is based on piece-square tables. `psq` must be strictly equal to
// `boardGetPsqScore(b)`.
score_pair_t boardUpdatePsqScore(const SoFCore::Board &b, SoFCore::Move move, score_pair_t psq);

}  // namespace SoFEval

#endif  // SOF_EVAL_EVALUATE_INCLUDED
