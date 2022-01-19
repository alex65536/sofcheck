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
#include <cstddef>
#include <utility>

#include "core/private/bit_consts.h"  // FIXME : refactor
#include "core/private/geometry.h"    // FIXME : refactor
#include "core/private/zobrist.h"     // FIXME : refactor
#include "eval/coefs.h"
#include "eval/private/bitboards.h"
#include "eval/private/pawn_cache.h"
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
using SoFCore::subcoord_t;

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

constexpr coef_t KING_ZONE_COSTS[8] = {0, 3, 2, 1, 0, 0, 0, 0};

constexpr bitboard_t BB_WHITE_SHIELDED_KING = 0xc300000000000000;
constexpr bitboard_t BB_BLACK_SHIELDED_KING = 0x00000000000000c3;

template <typename S>
typename Evaluator<S>::Tag Evaluator<S>::Tag::from(const Board &b) {
  using Weights = Private::Weights<S>;

  auto psq = Pair::from(S());
  uint32_t stage = 0;
  for (coord_t i = 0; i < 64; ++i) {
    psq += Weights::PSQ[b.cells[i]][i];
    stage += STAGES[b.cells[i]];
  }
  SoFCore::board_hash_t pawnHash = 0;
  for (coord_t i = 0; i < 64; ++i) {
    const cell_t cell = b.cells[i];
    if (cell == SoFCore::EMPTY_CELL || SoFCore::cellPiece(cell) != Piece::Pawn) {
      continue;
    }
    pawnHash ^= SoFCore::Private::g_zobristPieces[cell][i];
  }
  return Tag(std::move(psq), stage, pawnHash);
}

template <typename S>
typename Evaluator<S>::Tag Evaluator<S>::Tag::updated(const Board &b, const Move move) const {
  using Weights = Private::Weights<S>;

  auto result = *this;
  if (move.kind == MoveKind::Null) {
    return result;
  }
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
    result.pawnHash_ ^= SoFCore::Private::g_zobristPieces[srcCell][move.src];
    return result;
  }
  if (srcCell == makeCell(color, Piece::Pawn)) {
    result.pawnHash_ ^= SoFCore::Private::g_zobristPieces[srcCell][move.src];
    result.pawnHash_ ^= SoFCore::Private::g_zobristPieces[srcCell][move.dst];
  }
  if (dstCell == makeCell(invert(color), Piece::Pawn)) {
    result.pawnHash_ ^= SoFCore::Private::g_zobristPieces[dstCell][move.dst];
  }
  result.inner_ += Weights::PSQ[srcCell][move.dst];
  if (move.kind == MoveKind::Enpassant) {
    const coord_t pawnPos = enpassantPawnPos(color, move.dst);
    const cell_t enemyPawn = makeCell(invert(color), Piece::Pawn);
    result.inner_ -= Weights::PSQ[enemyPawn][pawnPos];
    result.pawnHash_ ^= SoFCore::Private::g_zobristPieces[enemyPawn][pawnPos];
    result.stage_ -= PAWN_STAGE;
  }
  return result;
}

template <typename S>
S Evaluator<S>::mix(const Pair &pair, const coef_t stage) {
  return static_cast<S>((pair.first() * stage + pair.second() * (256 - stage)) >> 8);
}

template <typename S>
template <typename W>
void Evaluator<S>::addWithCoef(S &result, const W &weight, coef_t coef) {
  result += static_cast<S>(weight * coef);
}

template <typename S>
Evaluator<S>::Evaluator() : pawnCache_(std::make_unique<Private::PawnCache<S>>()) {}

template <typename S>
Evaluator<S>::~Evaluator() = default;

template <typename S>
S Evaluator<S>::evalForWhite(const Board &b, const Tag &tag) {
  const auto rawStage = static_cast<coef_t>(((tag.stage_ << 8) + (TOTAL_STAGE >> 1)) / TOTAL_STAGE);
  const auto stage = std::min<coef_t>(rawStage, 256);

  auto result = mix(tag.inner_, stage);
  result += pawnCache_->get(tag.pawnHash_, [&]() { return evalPawns(b); });
  result += evalByColor<Color::White>(b, stage);
  result -= evalByColor<Color::Black>(b, stage);

  return result;
}

template <typename S>
S Evaluator<S>::evalPawns(const SoFCore::Board &b) {
  using Weights = Private::Weights<S>;

  const bitboard_t bbWhitePawns = b.bbPieces[makeCell(Color::White, Piece::Pawn)];
  const bitboard_t bbBlackPawns = b.bbPieces[makeCell(Color::Black, Piece::Pawn)];
  const bitboard_t bbAllPawns = bbWhitePawns | bbBlackPawns;
  const bitboard_t bbWhiteAttacks =
      ((SoFCore::Private::advancePawnLeft(Color::White,
                                          bbWhitePawns & ~SoFCore::Private::BB_COL[0])) |
       (SoFCore::Private::advancePawnRight(Color::White,
                                           bbWhitePawns & ~SoFCore::Private::BB_COL[7])));
  const bitboard_t bbBlackAttacks =
      ((SoFCore::Private::advancePawnLeft(Color::Black,
                                          bbBlackPawns & ~SoFCore::Private::BB_COL[0])) |
       (SoFCore::Private::advancePawnRight(Color::Black,
                                           bbBlackPawns & ~SoFCore::Private::BB_COL[7])));

  const auto doEvalPawns = [&](const Color c) {
    S result{};

    const bitboard_t bbPawns = (c == Color::White) ? bbWhitePawns : bbBlackPawns;
    const bitboard_t bbEnemyPawns = (c == Color::White) ? bbBlackPawns : bbWhitePawns;
    const bitboard_t bbAttacks = (c == Color::White) ? bbWhiteAttacks : bbBlackAttacks;
    const bitboard_t bbEnemyAttacks = (c == Color::White) ? bbBlackAttacks : bbWhiteAttacks;

    coef_t isolatedPawns = 0;
    coef_t doublePawns = 0;
    coef_t passedPawns = 0;
    coef_t openPawns = 0;
    coef_t candidatePawns = 0;

    const auto *bbOpenPawns =
        (c == Color::White) ? Private::BB_OPEN_PAWN_WHITE : Private::BB_OPEN_PAWN_BLACK;
    const auto *bbPassedPawns =
        (c == Color::White) ? Private::BB_PASSED_PAWN_WHITE : Private::BB_PASSED_PAWN_BLACK;
    const auto *attackFrontspans =
        (c == Color::White) ? Private::ATTACK_FRONTSPANS_WHITE : Private::ATTACK_FRONTSPANS_BLACK;

    bitboard_t bbIterPawns = bbPawns;
    bitboard_t bbAttackFrontspans = 0;
    while (bbIterPawns) {
      const auto src = static_cast<coord_t>(SoFUtil::extractLowest(bbIterPawns));
      if (!(bbPawns & Private::BB_ISOLATED_PAWN[src])) {
        ++isolatedPawns;
      }
      if (bbPawns & Private::BB_DOUBLE_PAWN[src]) {
        ++doublePawns;
      }
      if (!(bbAllPawns & bbOpenPawns[src])) {
        ++openPawns;
        if (!(bbEnemyPawns & bbPassedPawns[src])) {
          ++passedPawns;
        } else if (!(bbEnemyAttacks & ~bbAttacks & bbOpenPawns[src])) {
          ++candidatePawns;
        }
      }
      bbAttackFrontspans |= attackFrontspans[src];
    }
    openPawns -= candidatePawns;
    openPawns -= passedPawns;

    const auto protectedPawns = static_cast<coef_t>(SoFUtil::popcount(bbPawns & bbAttacks));
    const auto backwardPawns = static_cast<coef_t>(SoFUtil::popcount(
        SoFCore::Private::advancePawnForward(c, bbPawns) & bbEnemyAttacks & ~bbAttackFrontspans));

    addWithCoef(result, Weights::PAWN_ISOLATED, isolatedPawns);
    addWithCoef(result, Weights::PAWN_DOUBLE, doublePawns);
    addWithCoef(result, Weights::PAWN_PASSED, passedPawns);
    addWithCoef(result, Weights::PAWN_OPEN, openPawns);
    addWithCoef(result, Weights::PAWN_CANDIDATE, candidatePawns);
    addWithCoef(result, Weights::PAWN_PROTECTED, protectedPawns);
    addWithCoef(result, Weights::PAWN_BACKWARD, backwardPawns);

    return result;
  };

  return doEvalPawns(Color::White) - doEvalPawns(Color::Black);
}

template <typename S>
template <Color C>
S Evaluator<S>::evalByColor(const Board &b, const coef_t stage) {
  using Weights = Private::Weights<S>;

  // Calculate pairs of bishops
  S result{};
  if (SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Bishop)]) >= 2) {
    result += Weights::TWO_BISHOPS;
  }

  const bitboard_t bbPawns = b.bbPieces[makeCell(C, Piece::Pawn)];
  const bitboard_t bbKing = b.bbPieces[makeCell(C, Piece::King)];
  const bitboard_t bbEnemyKing = b.bbPieces[makeCell(invert(C), Piece::King)];
  const auto kingPos = static_cast<coord_t>(SoFUtil::getLowest(bbKing));
  const auto enemyKingPos = static_cast<coord_t>(SoFUtil::getLowest(bbEnemyKing));

  // Calculate pieces near to the opponent's king
  const auto generateNearPieces = [&](const Piece piece, const auto &weight) {
    const bitboard_t bb = b.bbPieces[makeCell(C, piece)];

    const auto countAtDistance = [&](const size_t dist) {
      return static_cast<coef_t>(
          SoFUtil::popcount(Private::KING_METRIC_RINGS[dist][enemyKingPos] & bb));
    };

    const coef_t nearCount = KING_ZONE_COSTS[1] * countAtDistance(1) +
                             KING_ZONE_COSTS[2] * countAtDistance(2) +
                             KING_ZONE_COSTS[3] * countAtDistance(3);

    addWithCoef(result, weight, nearCount);
  };

  generateNearPieces(Piece::Queen, Weights::QUEEN_NEAR_TO_KING);
  generateNearPieces(Piece::Rook, Weights::ROOK_NEAR_TO_KING);
  generateNearPieces(Piece::Bishop, Weights::BISHOP_NEAR_TO_KING);
  generateNearPieces(Piece::Rook, Weights::KNIGHT_NEAR_TO_KING);

  // Calculate king pawn shield
  constexpr bitboard_t bbShieldedKing =
      (C == Color::White) ? BB_WHITE_SHIELDED_KING : BB_BLACK_SHIELDED_KING;
  if (bbKing & bbShieldedKing) {
    const subcoord_t kingY = SoFCore::coordY(kingPos);

    const coord_t shift1 = (C == Color::White) ? (kingY + 47) : (kingY + 7);
    const coord_t shift2 = (C == Color::White) ? (kingY + 39) : (kingY + 15);

    constexpr bitboard_t row1 = SoFCore::Private::BB_ROW[(C == Color::White) ? 6 : 1];
    constexpr bitboard_t row2 = SoFCore::Private::BB_ROW[(C == Color::White) ? 5 : 2];

    const bitboard_t shieldMask1 = ((bbPawns & row1) >> shift1) & 7U;
    const bitboard_t shieldMask2 = ((bbPawns & row2) >> shift2) & 7U;

    const bool inverted = kingY > 4;
    const auto shieldWeights = inverted ? Weights::KING_PAWN_SHIELD_INV : Weights::KING_PAWN_SHIELD;
    const Pair kingPawnResult = shieldWeights[shieldMask1][shieldMask2];

    result += mix(kingPawnResult, stage);
  }

  return result;
}

// Template instantiations for all score types
template class Evaluator<score_t>;
template class Evaluator<Coefs>;

}  // namespace SoFEval
