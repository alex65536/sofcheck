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

#ifndef SOF_SEARCH_PRIVATE_CONSTS_INCLUDED
#define SOF_SEARCH_PRIVATE_CONSTS_INCLUDED

#include "eval/score.h"

namespace SoFSearch::Private {

// Maximum allowed depth
constexpr size_t MAX_DEPTH = 255;
// Size of the search stack, i. e. maximum allowed value of `idepth`, minus one
constexpr size_t MAX_STACK_DEPTH = MAX_DEPTH + 10;

// Constants for tuning null move heuristics
namespace NullMove {
// Minimum depth on which we can activate null move pruning
constexpr int32_t MIN_DEPTH = 5;
// Depth decrement when performing null move search, i. e. `R + 1` in chessprogramming.org
// terminology
constexpr int32_t DEPTH_DEC = 3;
// Depth decrement when null-move reduction is applied
constexpr int32_t REDUCTION_DEC = 4;

// Verify that we cannot reach `depth <= 0` after the reduction
static_assert(MIN_DEPTH > REDUCTION_DEC);
}  // namespace NullMove

// Constants for tuning futility pruning
namespace Futility {
// Maximum depth to activate futility pruning
constexpr int32_t MAX_DEPTH = 2;
// Futility margin (currenly, half a pawn)
constexpr SoFEval::score_t MARGIN = 50;
}  // namespace Futility

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_CONSTS_INCLUDED
