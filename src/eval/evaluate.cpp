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

ScorePair boardGetPsqScore(const Board &b) {
  auto result = ScorePair::from(0);
  for (coord_t i = 0; i < 64; ++i) {
    result += PIECE_SQUARE_TABLE[b.cells[i]][i];
  }
  return result;
}

ScorePair boardUpdatePsqScore(const Board &b, const Move move, ScorePair psq) {
  const Color color = b.side;
  if (move.kind == MoveKind::CastlingKingside) {
    return psq + SCORE_CASTLING_KINGSIDE_UPD[static_cast<size_t>(color)];
  }
  if (move.kind == MoveKind::CastlingQueenside) {
    return psq + SCORE_CASTLING_QUEENSIDE_UPD[static_cast<size_t>(color)];
  }
  const cell_t srcCell = b.cells[move.src];
  const cell_t dstCell = b.cells[move.dst];
  psq -= PIECE_SQUARE_TABLE[srcCell][move.src] + PIECE_SQUARE_TABLE[dstCell][move.dst];
  if (isMoveKindPromote(move.kind)) {
    return psq + PIECE_SQUARE_TABLE[makeCell(color, moveKindPromotePiece(move.kind))][move.dst];
  }
  psq += PIECE_SQUARE_TABLE[srcCell][move.dst];
  if (move.kind == MoveKind::Enpassant) {
    const coord_t pawnPos = enpassantPawnPos(color, move.dst);
    psq -= PIECE_SQUARE_TABLE[makeCell(invert(color), Piece::Pawn)][pawnPos];
  }
  return psq;
}

}  // namespace SoFEval
