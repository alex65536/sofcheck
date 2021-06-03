#ifndef SOF_SEARCH_PRIVATE_CONSTS_INCLUDED
#define SOF_SEARCH_PRIVATE_CONSTS_INCLUDED

#include "eval/score.h"

namespace SoFSearch::Private {

constexpr size_t MAX_DEPTH = 255;

constexpr SoFEval::score_t FUTILITY_THRESHOLD = 50;

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_CONSTS_INCLUDED
