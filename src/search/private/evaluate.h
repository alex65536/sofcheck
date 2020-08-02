#ifndef SOF_SEARCH_PRIVATE_EVALUATE_INCLUDED
#define SOF_SEARCH_PRIVATE_EVALUATE_INCLUDED

#include "core/board.h"
#include "core/move.h"
#include "search/private/score.h"

namespace SoFSearch::Private {

score_pair_t boardGetPsqScore(const SoFCore::Board &b);

score_pair_t boardUpdatePsqScore(const SoFCore::Board &b, const SoFCore::Move move,
                                score_pair_t score);

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_EVALUATE_INCLUDED
