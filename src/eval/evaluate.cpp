#include "eval/evaluate.h"

#include "eval/private/piece_square_table.h"
#include "util/misc.h"

namespace SoFEval {

using SoFCore::Board;
using SoFCore::cell_t;
using SoFCore::Color;
using SoFCore::coord_t;
using SoFCore::Move;
using SoFCore::MoveKind;
using SoFCore::Piece;

using SoFEval::Private::PIECE_SQUARE_TABLE;
using SoFEval::Private::SCORE_CASTLING_KINGSIDE_UPD;
using SoFEval::Private::SCORE_CASTLING_QUEENSIDE_UPD;

template <typename S>
typename Evaluator<S>::Tag Evaluator<S>::Tag::from(const SoFCore::Board &b) {
  auto result = ScorePair::from(0);
  for (coord_t i = 0; i < 64; ++i) {
    result += PIECE_SQUARE_TABLE[b.cells[i]][i];
  }
  return Tag(result);
}

template <typename S>
typename Evaluator<S>::Tag Evaluator<S>::Tag::updated(const SoFCore::Board &b,
                                                      const Move move) const {
  Pair psq = inner_;
  const Color color = b.side;
  if (move.kind == MoveKind::CastlingKingside) {
    return Tag(psq + SCORE_CASTLING_KINGSIDE_UPD[static_cast<size_t>(color)]);
  }
  if (move.kind == MoveKind::CastlingQueenside) {
    return Tag(psq + SCORE_CASTLING_QUEENSIDE_UPD[static_cast<size_t>(color)]);
  }
  const cell_t srcCell = b.cells[move.src];
  const cell_t dstCell = b.cells[move.dst];
  psq -= PIECE_SQUARE_TABLE[srcCell][move.src] + PIECE_SQUARE_TABLE[dstCell][move.dst];
  if (isMoveKindPromote(move.kind)) {
    return Tag(psq +
               PIECE_SQUARE_TABLE[makeCell(color, moveKindPromotePiece(move.kind))][move.dst]);
  }
  psq += PIECE_SQUARE_TABLE[srcCell][move.dst];
  if (move.kind == MoveKind::Enpassant) {
    const coord_t pawnPos = enpassantPawnPos(color, move.dst);
    psq -= PIECE_SQUARE_TABLE[makeCell(invert(color), Piece::Pawn)][pawnPos];
  }
  return Tag(psq);
}

// Template instantiations for all score types
template class Evaluator<score_t>;

}  // namespace SoFEval
