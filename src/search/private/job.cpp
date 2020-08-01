#include "search/private/job.h"

#include "core/movegen.h"
#include "search/private/types.h"

// TODO : remove these headers
#include <algorithm>

#include "core/strutil.h"
#include "core/types.h"
#include "util/bit.h"

namespace SoFSearch::Private {

using SoFCore::Board;
using SoFCore::Move;
using SoFCore::MovePersistence;

// This very-very simple alpha-beta search is here for testing only
// TODO : rewrite it completely!
score_t stupidAlphaBetaSearch(Board &board, const int8_t depth, score_t alpha, score_t beta,
                              Move *pv, size_t &pvLen, JobControl &control, JobStats &stats) {
  if (depth == 0) {
    pvLen = 0;
    using SoFCore::Color;
    using SoFCore::Piece;
    const Color us = board.side;
    const Color enemy = invert(us);
    score_t score = 0;
    score += 100 * SoFUtil::popcount(board.bbPieces[makeCell(us, Piece::Pawn)]);
    score += 320 * SoFUtil::popcount(board.bbPieces[makeCell(us, Piece::Knight)]);
    score += 330 * SoFUtil::popcount(board.bbPieces[makeCell(us, Piece::Bishop)]);
    score += 500 * SoFUtil::popcount(board.bbPieces[makeCell(us, Piece::Rook)]);
    score += 900 * SoFUtil::popcount(board.bbPieces[makeCell(us, Piece::Queen)]);
    score -= 100 * SoFUtil::popcount(board.bbPieces[makeCell(enemy, Piece::Pawn)]);
    score -= 320 * SoFUtil::popcount(board.bbPieces[makeCell(enemy, Piece::Knight)]);
    score -= 330 * SoFUtil::popcount(board.bbPieces[makeCell(enemy, Piece::Bishop)]);
    score -= 500 * SoFUtil::popcount(board.bbPieces[makeCell(enemy, Piece::Rook)]);
    score -= 900 * SoFUtil::popcount(board.bbPieces[makeCell(enemy, Piece::Queen)]);
    return score;
  }

  if (control.isStopped()) {
    return 0;
  }
  stats.incNodes();

  Move moves[300];
  bool hasMove = false;
  const size_t moveCnt = genAllMoves(board, moves);
  for (size_t i = 0; i < moveCnt; ++i) {
    const MovePersistence persistence = moveMake(board, moves[i]);
    if (!isMoveLegal(board)) {
      moveUnmake(board, moves[i], persistence);
      continue;
    }
    hasMove = true;
    Move newPv[128];
    size_t newPvLen;
    const score_t score =
        -stupidAlphaBetaSearch(board, depth - 1, -beta, -alpha, newPv, newPvLen, control, stats);
    moveUnmake(board, moves[i], persistence);
    if (score > alpha) {
      alpha = score;
      pv[0] = moves[i];
      std::copy(newPv, newPv + newPvLen, pv + 1);
      pvLen = newPvLen + 1;
    }
    if (alpha >= beta) {
      return alpha;
    }
  }

  if (!hasMove) {
    pvLen = 0;
    return isCheck(board) ? -30000 : 0;
  }
  return alpha;
}

void Job::run(Board board, const Move *moves, size_t count) {
  // TODO : use repetition table

  for (size_t i = 0; i < count; ++i) {
    moveMake(board, moves[i]);
  }

  Move bestMove = Move::null();
  for (int8_t depth = 1; depth < 127; ++depth) {
    Move pv[128];
    size_t pvLen;
    const score_t score =
        stupidAlphaBetaSearch(board, depth, -SCORE_INF, SCORE_INF, pv, pvLen, control_, stats_);
    if (control_.isStopped()) {
      server_->finishSearch(bestMove);
      return;
    }
    server_->sendResult({static_cast<size_t>(depth), pv, pvLen,
                         SoFBotApi::PositionCost::centipawns(score),
                         SoFBotApi::PositionCostBound::Exact});
    bestMove = pv[0];
  }

  server_->finishSearch(bestMove);
  control_.stop();
}

}  // namespace SoFSearch::Private
