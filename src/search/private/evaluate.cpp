#include "search/private/evaluate.h"

#include "search/private/piece_square_table.h"
#include "util/bit.h"
#include "util/misc.h"
#include "core/private/bit_consts.h"

namespace SoFSearch::Private {

using SoFCore::Board;
using SoFCore::cell_t;
using SoFCore::Color;
using SoFCore::coord_t;
using SoFCore::Move;
using SoFCore::MoveKind;
using SoFCore::Piece;
using SoFCore::bitboard_t;

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

constexpr int32_t TOTAL_STAGE =
    PAWN_STAGE * 16 + KNIGHT_STAGE * 4 + BISHOP_STAGE * 4 + ROOK_STAGE * 4 + QUEEN_STAGE * 2;

static_assert(TOTAL_STAGE % 2 == 0);

template<Color C>
inline static bool drawNode(const SoFCore::Board &b, const score_pair_t psq)
{
    Color D = invert(C);
    if (b.bbPieces[makeCell(C, Piece::Pawn)] != 0 ||
    b.bbPieces[makeCell(D, Piece::Pawn)] != 0) return false;  //is there are pawns on the board
    //we cannot say if it is a draw.
    if (SoFUtil::popcount(b.bbAll)==4)
    {
        //KR vs. KB is almost
        //always a draw; KR vs. KN isn't a draw: if a knight is not close to the king it can be possible to win.
        //8/8/8/3K4/R7/7n/8/1k6 w - - 0 1 as an example.
        if (SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Rook)]) == 1 &&
        SoFUtil::popcount(b.bbPieces[makeCell(D, Piece::Bishop)]) == 1) return true;
        //KNN vs. K is almost always a draw.
        if (SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Knight)]) == 2) return true;
        //100 is a pawn cost. We can check KN vs. KB, KN vs. KN and KB vs. KB in such way.
        if (abs(scorePairSecond(psq)) <= 100) return true;
    }
    if (SoFUtil::popcount(b.bbAll) == 5)
    {
        const size_t ourKnight = SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Knight)]);
        const size_t ourBishop = SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Bishop)]);
        const size_t ourRook = SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Rook)]);
        const size_t enemyKnight = SoFUtil::popcount(b.bbPieces[makeCell(D, Piece::Knight)]);
        const size_t enemyBishop = SoFUtil::popcount(b.bbPieces[makeCell(D, Piece::Bishop)]);
        //KNN vs. KN is draw
        if (ourKnight == 2 && enemyKnight == 1) return true;
        //KNN vs. KB is draw
        if (ourKnight == 2 && enemyBishop == 1) return true;
        //KNB vs. KN is draw
        if (ourKnight == 1 && ourBishop == 1 && enemyBishop == 1) return true;
        //KNB vs. KB is draw
        if (ourKnight == 1 && ourBishop == 1 && enemyKnight == 1) return true;
        //KBB vs. KB is draw
        //Note that KBB vs. KN or even KBB vs. KNN is not a draw.
        if (ourBishop == 2 && enemyBishop == 1) return true;
        //KR vs. KNB is draw
        if (ourRook == 1 && ourBishop == 1 && enemyKnight == 1) return true;
        //KR vs. KNN is draw
        if (ourRook == 1 && enemyKnight == 2) return true;
        //KR vs. KBB is draw
        if (ourRook == 1 && enemyBishop == 2) return true;
    }
    if (SoFUtil::popcount(b.bbAll)==6)
    {
        //KNN vs. KNN is draw
        if (SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Knight)]) == 2 &&
        SoFUtil::popcount(b.bbPieces[makeCell(D, Piece::Knight)]) == 2) return true;
        //Note that KNNN vs. KN is not a draw.
        //Note also that KBB vs. KNN is not a draw.
        //KNNN vs. KR is a draw, and KNNN vs. KB is not. This cases are not included, because NNN is too
        //rare to be looked through here.
    }
    return false;
}

//Here are all constants for the evaluation
constexpr score_t BISHOP_PAIR[TOTAL_STAGE/2+1]={0,100,90,80,70,60,50,40,30,20,10,5,0};  //Pairs of bishops
//are evaluated more in endspiel, and less in the early mittelspiel.
constexpr score_t ROOK_PAIR=-25;  //This is called "principle of redundancy" by Larry Kaufman. It
//allows engine to understand that NB-RP is less that it learns from piece costs.
constexpr score_t KNIGHT_PAIR=-25;  //This value is const, because BISHOP_PAIR is not const; NN is
//almost always weaker than NB or BB.
constexpr score_t NO_PAWN=-300;  //It is much harder to win without pawns; in fact,
//with almost equal material it is most commonly impossible.

template<Color C>
inline static score_t getMaterialEvaluation(const SoFCore::Board &b, int32_t evaluationStage)
{
    score_t value = 0;
    const bitboard_t bishops=SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Bishop)]);
    if ((bishops & SoFCore::Private::BB_CELLS_WHITE) != 0 && (bishops & SoFCore::Private::BB_CELLS_BLACK) !=0 )
    {
        value+=BISHOP_PAIR[evaluationStage/2];
    }
    if (SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Rook)])>=2) value+=ROOK_PAIR;
    if (SoFUtil::popcount(b.bbPieces[makeCell(C, Piece::Knight)])>=2) value+=KNIGHT_PAIR;
    if (b.bbPieces[makeCell(C, Piece::Pawn)] == 0) value+=NO_PAWN;
    return value;
}

inline static score_t taperedEval(const score_pair_t score, const int32_t stage)
{
    return (((scorePairFirst(score) * (256 - stage)) + (scorePairSecond(score) * stage)) >> 8);
}

inline static bool lazyEval(const score_t alpha, const score_t beta, const score_t cost, const score_t diff)
{
    return cost + diff <= alpha || cost - diff >= beta;
}

inline static score_t doEvaluate(const SoFCore::Board &b, const score_pair_t psq, const score_t alpha, const score_t beta) {
  //First we should check some positions, that has a chance of being winned, but in fact it is draw.
  if (drawNode<Color::White>(b, psq)) return 0;
  if (drawNode<Color::Black>(b, psq)) return 0;
  // Calculating game stage: this is a number from 0 to 256, denoting the stage of the game: 0 is
  // the start of the game
  int32_t stage = TOTAL_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Pawn)]) * PAWN_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Knight)]) * KNIGHT_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Bishop)]) * BISHOP_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Rook)]) * ROOK_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::White, Piece::Queen)]) * QUEEN_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Pawn)]) * PAWN_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Knight)]) * KNIGHT_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Bishop)]) * BISHOP_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Rook)]) * ROOK_STAGE;
  stage -= SoFUtil::popcount(b.bbPieces[makeCell(Color::Black, Piece::Queen)]) * QUEEN_STAGE;
  stage = std::max(stage, 0);
  int32_t evaluationStage=stage;
  stage = (stage * 256 + (TOTAL_STAGE / 2)) / TOTAL_STAGE;
  // Calculating all dynamic values
  score_t value = taperedEval(psq, stage);
  //First lazy evaluation
  if (lazyEval(alpha, beta, value, 900)) return value <= alpha ? alpha : beta;
  const score_t materialValue =
  getMaterialEvaluation<Color::White>(b, evaluationStage) - getMaterialEvaluation<Color::Black>(b, evaluationStage);
  value+=materialValue;
  if (lazyEval(alpha, beta, value, 300)) return (value <= alpha) ? alpha : beta;
  //Second lazy evaluation
  return value;
}

score_t evaluate(const SoFCore::Board &b, const score_pair_t psq, const score_t alpha, const score_t beta) {
  const Color side = b.side;
  score_t value=doEvaluate(b, psq, (side == Color::White) ? alpha : -beta, (side == Color::White) ? beta : -alpha);
  if (side == Color::Black)
  {
    value*=-1;
  }
  return value;
}

}  // namespace SoFSearch::Private
