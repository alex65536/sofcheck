#ifndef SOF_SEARCH_PRIVATE_TYPES_INCLUDED
#define SOF_SEARCH_PRIVATE_TYPES_INCLUDED

#include <cstdint>
#include <limits>

namespace SoFSearch::Private {

// Position score (in centipawns)
using score_t = int16_t;

// We need to ensure that `-SCORE_INF` is stll a valid score value
constexpr score_t SCORE_INF = 32767;

// TODO : improve this and include mate score

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_TYPES_INCLUDED
