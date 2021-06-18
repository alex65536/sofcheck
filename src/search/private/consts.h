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

constexpr size_t MAX_DEPTH = 255;

constexpr int32_t NULL_MOVE_MIN_DEPTH = 5;
constexpr int32_t NULL_MOVE_DEPTH_DEC = 3;
constexpr int32_t NULL_MOVE_REDUCTION_DEC = 4;

static_assert(NULL_MOVE_MIN_DEPTH > NULL_MOVE_REDUCTION_DEC);

constexpr SoFEval::score_t FUTILITY_THRESHOLD = 50;

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_CONSTS_INCLUDED
