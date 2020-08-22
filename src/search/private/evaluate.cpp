#include "search/private/evaluate.h"

#include "search/private/piece_square_table.h"
#include "util/bit.h"
#include "util/misc.h"

namespace SoFSearch::Private {

using SoFCore::Board;
using SoFCore::cell_t;
using SoFCore::Color;
using SoFCore::coord_t;
using SoFCore::Move;
using SoFCore::MoveKind;
using SoFCore::Piece;

score_pair_t boardGetPsqScore(const Board &b) {
  score_pair_t result = 0;
  for (coord_t i = 0; i < 64; ++i) {
    result += PIECE_SQUARE_TABLE[b.cells[i]][i];
  }
  return result;
}

score_pair_t boardUpdatePsqScore(const Board &b, const Move move, score_pair_t psq) {
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

constexpr int32_t PAWN_STAGE = 0;
constexpr int32_t KNIGHT_STAGE = 1;
constexpr int32_t BISHOP_STAGE = 1;
constexpr int32_t ROOK_STAGE = 2;
constexpr int32_t QUEEN_STAGE = 4;

constexpr int32_t TOTAL_STAGE = PAWN_STAGE*16 + KNIGHT_STAGE * 4 + BISHOP_STAGE * 4 + ROOK_STAGE * 4 + QUEEN_STAGE * 2;

score_t evaluate(const SoFCore::Board &b, const score_pair_t psq) {
  score_t valueMid = scorePairFirst(psq);
  score_t valueEnd = scorePairSecond(psq);
  //Calculating game stage: this is a number from 0 to 256, denoting the stage of the game: 0 is the start of the game
  int32_t stage=TOTAL_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Pawn)])*PAWN_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Knight)])*KNIGHT_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Bishop)])*BISHOP_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Rook)])*ROOK_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Queen)])*QUEEN_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Pawn)])*PAWN_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Knight)])*KNIGHT_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Bishop)])*BISHOP_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Rook)])*ROOK_STAGE;
  stage-=SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Queen)])*QUEEN_STAGE;
  stage = (stage * 256 + (TOTAL_STAGE / 2)) / TOTAL_STAGE;
  //Calculating all dynamic values
  int32_t value=((valueMid * (256 - stage)) + (valueEnd*stage))/256;
  if (b.side == Color::Black) {
    value *=-1;
  }
  return value;
}

}  // namespace SoFSearch::Private
