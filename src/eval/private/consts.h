// This file is part of SoFCheck
//
// Copyright (c) 2022 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_EVAL_PRIVATE_CONSTS_INCLUDED
#define SOF_EVAL_PRIVATE_CONSTS_INCLUDED

#include <cstdint>

#include "eval/coefs.h"

namespace SoFEval::Private {

// Game stage related constants
constexpr uint32_t STAGE_PAWN = 0;
constexpr uint32_t STAGE_KNIGHT = 1;
constexpr uint32_t STAGE_BISHOP = 1;
constexpr uint32_t STAGE_ROOK = 2;
constexpr uint32_t STAGE_QUEEN = 4;
constexpr uint32_t STAGE_TOTAL =
    16 * STAGE_PAWN + 4 * STAGE_KNIGHT + 4 * STAGE_BISHOP + 4 * STAGE_ROOK + 2 * STAGE_QUEEN;
constexpr uint32_t STAGES[15] = {
    0, STAGE_PAWN, 0, STAGE_KNIGHT, STAGE_BISHOP, STAGE_ROOK, STAGE_QUEEN, 0,
    0, STAGE_PAWN, 0, STAGE_KNIGHT, STAGE_BISHOP, STAGE_ROOK, STAGE_QUEEN};

// Importance of various distances between an attacking piece and the king
constexpr coef_t KING_ZONE_COST1 = 5;
constexpr coef_t KING_ZONE_COST2 = 4;
constexpr coef_t KING_ZONE_COST3 = 1;

}  // namespace SoFEval::Private

#endif  // SOF_EVAL_PRIVATE_CONSTS_INCLUDED
