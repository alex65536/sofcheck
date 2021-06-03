#include "eval/evaluate.h"

#include <algorithm>
#include <utility>

#include "eval/coefs.h"
#include "eval/private/weights.h"
#include "eval/score.h"
#include "util/bit.h"

namespace SoFEval {

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
S Evaluator<S>::evalForWhite(const SoFCore::Board &b, const Tag tag) {
  using Weights = Private::Weights<S>;

  const uint32_t rawStage = ((tag.stage_ << 8) + (TOTAL_STAGE >> 1)) / TOTAL_STAGE;
  const uint32_t stage = std::min<uint32_t>(rawStage, 256);

  S result = (tag.inner_.first() * stage + tag.inner_.second() * (256 - stage)) >> 8;
  // TODO : do not write the same code for white and black twice
  if (SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Bishop)]) >= 2) {
    result += Weights::TWO_BISHOPS;
  }
  if (SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Bishop)]) >= 2) {
    result -= Weights::TWO_BISHOPS;
  }

  return result;
}

// Template instantiations for all score types
template class Evaluator<score_t>;
template class Evaluator<Coefs>;

}  // namespace SoFEval
