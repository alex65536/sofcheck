#include "eval/evaluate.h"

#include "eval/private/weights.h"
#include "eval/coefs.h"
#include "eval/score.h"
#include "util/misc.h"

namespace SoFEval {

using SoFCore::Board;
using SoFCore::cell_t;
using SoFCore::Color;
using SoFCore::coord_t;
using SoFCore::Move;
using SoFCore::MoveKind;
using SoFCore::Piece;

template <typename S>
typename Evaluator<S>::Tag Evaluator<S>::Tag::from(const SoFCore::Board &b) {
  using Weights = Private::Weights<S>;

  auto result = Pair::from(S());
  for (coord_t i = 0; i < 64; ++i) {
    result += Weights::PSQ[b.cells[i]][i];
  }
  return Tag(result);
}

template <typename S>
typename Evaluator<S>::Tag Evaluator<S>::Tag::updated(const SoFCore::Board &b,
                                                      const Move move) const {
  using Weights = Private::Weights<S>;

  Pair psq = inner_;
  const Color color = b.side;
  if (move.kind == MoveKind::CastlingKingside) {
    return Tag(psq + Weights::PSQ_KINGSIDE_UPD[static_cast<size_t>(color)]);
  }
  if (move.kind == MoveKind::CastlingQueenside) {
    return Tag(psq + Weights::PSQ_QUEENSIDE_UPD[static_cast<size_t>(color)]);
  }
  const cell_t srcCell = b.cells[move.src];
  const cell_t dstCell = b.cells[move.dst];
  psq -= Weights::PSQ[srcCell][move.src] + Weights::PSQ[dstCell][move.dst];
  if (isMoveKindPromote(move.kind)) {
    return Tag(psq +
               Weights::PSQ[makeCell(color, moveKindPromotePiece(move.kind))][move.dst]);
  }
  psq += Weights::PSQ[srcCell][move.dst];
  if (move.kind == MoveKind::Enpassant) {
    const coord_t pawnPos = enpassantPawnPos(color, move.dst);
    psq -= Weights::PSQ[makeCell(invert(color), Piece::Pawn)][pawnPos];
  }
  return Tag(psq);
}

// Template instantiations for all score types
template class Evaluator<score_t>;
template class Evaluator<Coefs>;

}  // namespace SoFEval
