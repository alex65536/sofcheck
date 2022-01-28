// This file is part of SoFCheck
//
// Copyright (c) 2020-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#include "core/bitboard.h"
#include "eval/coefs.h"
#include "eval/private/bitboard.h"
#include "eval/private/cache.h"
#include "eval/private/consts.h"
#include "eval/private/weights.h"
#include "eval/score.h"
#include "util/bit.h"
#include "util/hash.h"

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

template <typename S>
typename Evaluator<S>::Tag Evaluator<S>::Tag::from(const Board &b) {
  using Weights = Private::Weights<S>;

  auto psq = Pair::from(S());
  uint32_t stage = 0;
  for (coord_t i = 0; i < 64; ++i) {
    psq += Weights::PSQ[b.cells[i]][i];
    stage += Private::STAGES[b.cells[i]];
  }
  return Tag(std::move(psq), stage);
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
    result.psqCost_ += Weights::PSQ_KINGSIDE_UPD[static_cast<size_t>(color)];
    return result;
  }
  if (move.kind == MoveKind::CastlingQueenside) {
    result.psqCost_ += Weights::PSQ_QUEENSIDE_UPD[static_cast<size_t>(color)];
    return result;
  }
  const cell_t srcCell = b.cells[move.src];
  const cell_t dstCell = b.cells[move.dst];
  result.psqCost_ -= Weights::PSQ[srcCell][move.src] + Weights::PSQ[dstCell][move.dst];
  result.stage_ -= Private::STAGES[dstCell];
  if (isMoveKindPromote(move.kind)) {
    const cell_t promoteCell = makeCell(color, moveKindPromotePiece(move.kind));
    result.psqCost_ += Weights::PSQ[promoteCell][move.dst];
    result.stage_ += Private::STAGES[promoteCell] - Private::STAGE_PAWN;
    return result;
  }
  result.psqCost_ += Weights::PSQ[srcCell][move.dst];
  if (move.kind == MoveKind::Enpassant) {
    const coord_t pawnPos = enpassantPawnPos(color, move.dst);
    const cell_t enemyPawn = makeCell(invert(color), Piece::Pawn);
    result.psqCost_ -= Weights::PSQ[enemyPawn][pawnPos];
    result.stage_ -= Private::STAGE_PAWN;
  }
  return result;
}

template <typename S>
class Evaluator<S>::Impl {
public:
  using Tag = Evaluator<S>::Tag;

  Impl(const Evaluator<S> &parent, const Board &b, Tag tag)
      : pawnCache_(*parent.pawnCache_), b_(b), tag_(std::move(tag)), stage_(calcStage(tag_)) {}

  inline S evalForWhite() {
    auto result = mix(tag_.psqCost_);
    const Private::hash_t pawnHash = calcPawnHash();
    pawnCache_.prefetch(pawnHash);
    result += evalKingSafety<Color::White>() - evalKingSafety<Color::Black>();
    result += evalMaterial<Color::White>() - evalMaterial<Color::Black>();
    const auto pawnValue = pawnCache_.get(pawnHash, [&]() { return evalPawns(); });
    result += pawnValue.score;
    result += evalOpenLines<Color::White>(pawnValue) - evalOpenLines<Color::Black>(pawnValue);
    return result;
  }

private:
  using Weights = Private::Weights<S>;

  // Given a pair `pair` of midgame and endgame score, and current game stage `stage`, calculate
  // the real score
  inline S mix(const Pair &pair) const {
    const coef_t stage = stage_;
    return static_cast<S>((pair.first() * stage + pair.second() * (COEF_UNIT - stage)) >>
                          COEF_UNIT_SHIFT);
  }

  // Helper function to add `weight * coef` to `result`. This function is mostly needed to silence
  // MSVC warnings about narrowing type conversions
  template <typename W>
  inline static void addWithCoef(S &result, const W &weight, const coef_t coef) {
    result += static_cast<S>(weight * coef);
  }

  // Calculates hash which is used as a key in pawn hash table
  inline Private::hash_t calcPawnHash() const {
    return SoFUtil::hash16(b_.bbPieces[makeCell(Color::White, Piece::Pawn)],
                           b_.bbPieces[makeCell(Color::Black, Piece::Pawn)]);
  }

  // Calculates the game stage (as a value in range from `0` to `COEF_UNIT`) from tag
  inline static coef_t calcStage(const Tag &tag) {
    const auto rawStage = static_cast<coef_t>(
        ((tag.stage_ << COEF_UNIT_SHIFT) + (Private::STAGE_TOTAL >> 1)) / Private::STAGE_TOTAL);
    return std::min<coef_t>(rawStage, COEF_UNIT);
  }

  inline Private::PawnCacheValue<S> evalPawns() const {
    const bitboard_t bbWhitePawns = b_.bbPieces[makeCell(Color::White, Piece::Pawn)];
    const bitboard_t bbBlackPawns = b_.bbPieces[makeCell(Color::Black, Piece::Pawn)];
    const bitboard_t bbAllPawns = bbWhitePawns | bbBlackPawns;
    const bitboard_t bbWhiteAttacks = SoFCore::advancePawnLeft(Color::White, bbWhitePawns) |
                                      SoFCore::advancePawnRight(Color::White, bbWhitePawns);
    const bitboard_t bbBlackAttacks = SoFCore::advancePawnLeft(Color::Black, bbBlackPawns) |
                                      SoFCore::advancePawnRight(Color::Black, bbBlackPawns);

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
      const auto *attackFrontspans = (c == Color::White) ? Private::BB_ATTACK_FRONTSPANS_WHITE
                                                         : Private::BB_ATTACK_FRONTSPANS_BLACK;

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
          SoFCore::advancePawnForward(c, bbPawns) & bbEnemyAttacks & ~bbAttackFrontspans));

      addWithCoef(result, Weights::PAWN_ISOLATED, isolatedPawns);
      addWithCoef(result, Weights::PAWN_DOUBLE, doublePawns);
      addWithCoef(result, Weights::PAWN_PASSED, passedPawns);
      addWithCoef(result, Weights::PAWN_OPEN, openPawns);
      addWithCoef(result, Weights::PAWN_CANDIDATE, candidatePawns);
      addWithCoef(result, Weights::PAWN_PROTECTED, protectedPawns);
      addWithCoef(result, Weights::PAWN_BACKWARD, backwardPawns);

      return result;
    };

    S score = doEvalPawns(Color::White) - doEvalPawns(Color::Black);
    const uint8_t bbWhiteCols = SoFUtil::byteGather(bbWhitePawns);
    const uint8_t bbBlackCols = SoFUtil::byteGather(bbBlackPawns);
    const uint8_t bbOpenCols = ~bbWhiteCols & ~bbBlackCols;
    const uint8_t bbWhiteOnlyCols = bbWhiteCols & ~bbBlackCols;
    const uint8_t bbBlackOnlyCols = ~bbWhiteCols & bbBlackCols;

    return Private::PawnCacheValue<S>::from(bbOpenCols, bbWhiteOnlyCols, bbBlackOnlyCols,
                                            std::move(score));
  }

  template <Color C>
  inline S evalKingSafety() const {
    S result{};

    const bitboard_t bbKing = b_.bbPieces[makeCell(C, Piece::King)];
    const auto kingPos = static_cast<coord_t>(SoFUtil::getLowest(bbKing));

    // Enemy pieces near king
    const auto generateNearPieces = [&](const Piece piece, const auto &weight) {
      const bitboard_t bb = b_.bbPieces[makeCell(invert(C), piece)];

      const auto countAtDistance = [&](const size_t dist) {
        return static_cast<coef_t>(
            SoFUtil::popcount(Private::BB_KING_METRIC_RING[kingPos][dist] & bb));
      };

      const coef_t nearCount = Private::KING_ZONE_COST1 * countAtDistance(1) +
                               Private::KING_ZONE_COST2 * countAtDistance(2) +
                               Private::KING_ZONE_COST3 * countAtDistance(3);

      addWithCoef(result, weight, nearCount);
    };

    generateNearPieces(Piece::Queen, Weights::QUEEN_NEAR_TO_KING);
    generateNearPieces(Piece::Rook, Weights::ROOK_NEAR_TO_KING);

    // Pawn shield and pawn storm
    constexpr bitboard_t bbShieldedKing =
        (C == Color::White) ? Private::BB_WHITE_SHIELDED_KING : Private::BB_BLACK_SHIELDED_KING;

    if (bbKing & bbShieldedKing) {
      const bitboard_t bbPawns = b_.bbPieces[makeCell(C, Piece::Pawn)];
      const bitboard_t bbEnemyPawns = b_.bbPieces[makeCell(invert(C), Piece::Pawn)];
      const subcoord_t kingY = SoFCore::coordY(kingPos);

      const coord_t shift1 = (C == Color::White) ? (kingY + 47) : (kingY + 7);
      const coord_t shift2 = (C == Color::White) ? (kingY + 39) : (kingY + 15);
      const coord_t shift3 = (C == Color::White) ? (kingY + 31) : (kingY + 23);

      constexpr bitboard_t row1 = SoFCore::BB_ROW[(C == Color::White) ? 6 : 1];
      constexpr bitboard_t row2 = SoFCore::BB_ROW[(C == Color::White) ? 5 : 2];
      constexpr bitboard_t row3 = SoFCore::BB_ROW[(C == Color::White) ? 4 : 3];

      const bitboard_t shieldMask1 = ((bbPawns & row1) >> shift1) & 7U;
      const bitboard_t shieldMask2 = ((bbPawns & row2) >> shift2) & 7U;

      const bitboard_t stormMask2 = ((bbEnemyPawns & row2) >> shift2) & 7U;
      const bitboard_t stormMask3 = ((bbEnemyPawns & row3) >> shift3) & 7U;

      const bool inverted = kingY > 4;
      const auto shieldWeights =
          inverted ? Weights::KING_PAWN_SHIELD_INV : Weights::KING_PAWN_SHIELD;
      const auto stormWeights = inverted ? Weights::KING_PAWN_STORM_INV : Weights::KING_PAWN_STORM;
      const Pair kingPawnResult =
          shieldWeights[shieldMask1][shieldMask2] + stormWeights[stormMask2][stormMask3];

      result += mix(kingPawnResult);
    }

    return result;
  }

  template <Color C>
  inline S evalMaterial() const {
    S result{};

    // Bishop pair
    if (SoFUtil::popcount(b_.bbPieces[makeCell(C, Piece::Bishop)]) >= 2) {
      result += Weights::TWO_BISHOPS;
    }

    return result;
  }

  template <Color C>
  inline S evalOpenLines(const Private::PawnCacheValue<S> &pawnValue) const {
    S result{};

    // Open and semi-open columns
    const bitboard_t bbOpenCols = SoFUtil::byteScatter(pawnValue.bbOpenCols);
    const bitboard_t bbSemiOpenCols = SoFUtil::byteScatter(
        (C == Color::White) ? pawnValue.bbBlackOnlyCols : pawnValue.bbWhiteOnlyCols);
    const bitboard_t bbRooks = b_.bbPieces[makeCell(C, Piece::Rook)];
    const bitboard_t bbQueens = b_.bbPieces[makeCell(C, Piece::Queen)];

    addWithCoef(result, Weights::ROOK_OPEN_COL,
                static_cast<coef_t>(SoFUtil::popcount(bbOpenCols & bbRooks)));
    addWithCoef(result, Weights::ROOK_SEMI_OPEN_COL,
                static_cast<coef_t>(SoFUtil::popcount(bbSemiOpenCols & bbRooks)));
    addWithCoef(result, Weights::QUEEN_OPEN_COL,
                static_cast<coef_t>(SoFUtil::popcount(bbOpenCols & bbQueens)));
    addWithCoef(result, Weights::QUEEN_SEMI_OPEN_COL,
                static_cast<coef_t>(SoFUtil::popcount(bbSemiOpenCols & bbQueens)));

    return result;
  }

  Private::PawnCache<S> &pawnCache_;
  const Board &b_;
  const Tag tag_;

  // Precalculated values
  const coef_t stage_;
};

template <typename S>
Evaluator<S>::Evaluator() : pawnCache_(std::make_unique<Private::PawnCache<S>>()) {}

template <typename S>
Evaluator<S>::~Evaluator() = default;

template <typename S>
Evaluator<S>::Evaluator(Evaluator &&) = default;

template <typename S>
Evaluator<S> &Evaluator<S>::operator=(Evaluator &&) = default;

template <typename S>
S Evaluator<S>::evalForWhite(const Board &b, const Tag &tag) const {
  return Impl(*this, b, tag).evalForWhite();
}

// Template instantiations for all score types
template class Evaluator<score_t>;
template class Evaluator<Coefs>;

}  // namespace SoFEval
