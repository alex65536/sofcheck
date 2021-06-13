// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

#include "eval/evaluate.h"

#include <algorithm>
#include <utility>

#include "eval/coefs.h"
#include "eval/private/metric.h"
#include "eval/private/weights.h"
#include "eval/score.h"
#include "util/bit.h"

namespace SoFEval {

using SoFCore::bitboard_t;
using SoFCore::Board;
using SoFCore::cell_t;
using SoFCore::Color;
using SoFCore::coord_t;
using SoFCore::Move;
using SoFCore::MoveKind;
using SoFCore::Piece;

constexpr uint32_t PAWN_STAGE = 0;
constexpr uint32_t KNIGHT_STAGE = 1;
constexpr uint32_t BISHOP_STAGE = 1;
constexpr uint32_t ROOK_STAGE = 2;
constexpr uint32_t QUEEN_STAGE = 4;
constexpr uint32_t TOTAL_STAGE =
    16 * PAWN_STAGE + 4 * KNIGHT_STAGE + 4 * BISHOP_STAGE + 4 * ROOK_STAGE + 2 * QUEEN_STAGE;
constexpr uint32_t STAGES[15] = {
    0, PAWN_STAGE, 0, KNIGHT_STAGE, BISHOP_STAGE, ROOK_STAGE, QUEEN_STAGE, 0,
    0, PAWN_STAGE, 0, KNIGHT_STAGE, BISHOP_STAGE, ROOK_STAGE, QUEEN_STAGE};

constexpr coef_t QUEEN_DISTANCES[8] = {0, 3, 2, 1, 0, 0, 0, 0};

template <typename S>
typename Evaluator<S>::Tag Evaluator<S>::Tag::from(const Board &b) {
  using Weights = Private::Weights<S>;

  auto psq = Pair::from(S());
  uint32_t stage = 0;
  for (coord_t i = 0; i < 64; ++i) {
    psq += Weights::PSQ[b.cells[i]][i];
    stage += STAGES[b.cells[i]];
  }
  return Tag(std::move(psq), stage);
}

template <typename S>
typename Evaluator<S>::Tag Evaluator<S>::Tag::updated(const Board &b, const Move move) const {
  using Weights = Private::Weights<S>;

  auto result = *this;
  const Color color = b.side;
  if (move.kind == MoveKind::CastlingKingside) {
    result.inner_ += Weights::PSQ_KINGSIDE_UPD[static_cast<size_t>(color)];
    return result;
  }
  if (move.kind == MoveKind::CastlingQueenside) {
    result.inner_ += Weights::PSQ_QUEENSIDE_UPD[static_cast<size_t>(color)];
    return result;
  }
  const cell_t srcCell = b.cells[move.src];
  const cell_t dstCell = b.cells[move.dst];
  result.inner_ -= Weights::PSQ[srcCell][move.src] + Weights::PSQ[dstCell][move.dst];
  result.stage_ -= STAGES[dstCell];
  if (isMoveKindPromote(move.kind)) {
    const cell_t promoteCell = makeCell(color, moveKindPromotePiece(move.kind));
    result.inner_ += Weights::PSQ[promoteCell][move.dst];
    result.stage_ += STAGES[promoteCell] - PAWN_STAGE;
    return result;
  }
  result.inner_ += Weights::PSQ[srcCell][move.dst];
  if (move.kind == MoveKind::Enpassant) {
    const coord_t pawnPos = enpassantPawnPos(color, move.dst);
    result.inner_ -= Weights::PSQ[makeCell(invert(color), Piece::Pawn)][pawnPos];
    result.stage_ -= PAWN_STAGE;
  }
  return result;
}

template <typename S>
S Evaluator<S>::evalForWhite(const Board &b, const Tag &tag) {
  const uint32_t rawStage = ((tag.stage_ << 8) + (TOTAL_STAGE >> 1)) / TOTAL_STAGE;
  const uint32_t stage = std::min<uint32_t>(rawStage, 256);

  S result = (tag.inner_.first() * stage + tag.inner_.second() * (256 - stage)) >> 8;

  result += evalByColor<Color::White>(b);
  result -= evalByColor<Color::Black>(b);

  return result;
}

template <typename S>
template <Color C>
S Evaluator<S>::evalByColor(const Board &b) {
  using Weights = Private::Weights<S>;

  // Calculate pairs of bishops
  S result{};
  if (SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Bishop)]) >= 2) {
    result += Weights::TWO_BISHOPS;
  }

  // Calculate queens near to opponent's king
  const coord_t king = SoFUtil::getLowest(b.bbPieces[makeCell(invert(C), Piece::King)]);
  const size_t bbQueen = b.bbPieces[makeCell(C, Piece::Queen)];
  coef_t nearCount = 0;
  for (size_t i = 1; i < 8; ++i) {
    nearCount +=
        QUEEN_DISTANCES[i] * SoFUtil::popcount(Private::KING_METRIC_RINGS[i][king] & bbQueen);
  }
  result += Weights::QUEEN_NEAR_TO_KING * nearCount;

  return result;
}

// Template instantiations for all score types
template class Evaluator<score_t>;
template class Evaluator<Coefs>;

}  // namespace SoFEval
