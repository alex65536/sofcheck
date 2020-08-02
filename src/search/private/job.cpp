#include "search/private/job.h"

#include "core/movegen.h"
#include "search/private/evaluate.h"
#include "search/private/move_picker.h"
#include "search/private/score.h"
#include "search/private/types.h"

// TODO : remove these headers
#include <algorithm>

#include "core/strutil.h"
#include "core/types.h"
#include "util/bit.h"
#include "util/logging.h"

namespace SoFSearch::Private {

using SoFCore::Board;
using SoFCore::Move;
using SoFCore::MovePersistence;
using SoFUtil::logError;

// Type of log entry
constexpr const char *SEARCH_JOB = "Search job";

struct StupidSearchStackFrame {
  KillerLine killers;
};

struct StupidSearchState {
  JobControl &control;
  JobStats &stats;
  StupidSearchStackFrame stack[128];
  HistoryTable history;

  StupidSearchState(JobControl &control, JobStats &stats) : control(control), stats(stats) {}
};

// This very-very simple alpha-beta search is here for testing only
// TODO : rewrite it completely!
score_t stupidAlphaBetaSearch(Board &board, const int8_t depth, const int8_t idepth, score_t alpha,
                              score_t beta, score_pair_t psq, Move *pv, size_t &pvLen,
                              StupidSearchState &state) {
  pvLen = 0;

  if (depth == 0) {
    const score_t score = scorePairFirst(psq);
    return (board.side == SoFCore::Color::White) ? score : -score;
  }

  if (state.control.isStopped()) {
    return 0;
  }
  state.stats.incNodes();

  bool hasMove = false;
  MovePicker picker(board, Move::null(), state.stack[idepth].killers, state.history);
  for (Move move = picker.next(); move != Move::invalid(); move = picker.next()) {
    if (move == Move::null()) {
      continue;
    }
    const score_pair_t newPsq = boardUpdatePsqScore(board, move, psq);
    const MovePersistence persistence = moveMake(board, move);
    if (!isMoveLegal(board)) {
      moveUnmake(board, move, persistence);
      continue;
    }
    hasMove = true;
    Move newPv[128];
    size_t newPvLen;
    const score_t score = -stupidAlphaBetaSearch(board, depth - 1, idepth + 1, -beta, -alpha,
                                                 newPsq, newPv, newPvLen, state);
    moveUnmake(board, move, persistence);
    if (score > alpha) {
      if (picker.stage() >= MovePickerStage::Killer) {
        state.stack[idepth].killers.add(move);
        ++state.history[move];
      }
      alpha = score;
      pv[0] = move;
      std::copy(newPv, newPv + newPvLen, pv + 1);
      pvLen = newPvLen + 1;
    }
    if (alpha >= beta) {
      return alpha;
    }
  }

  if (!hasMove) {
    pvLen = 0;
    return isCheck(board) ? scoreCheckmateLose(idepth) : 0;
  }
  return alpha;
}

void Job::run(Board board, const Move *moves, size_t count) {
  // TODO : use repetition table

  for (size_t i = 0; i < count; ++i) {
    moveMake(board, moves[i]);
  }

  StupidSearchState state(control_, stats_);
  Move bestMove = Move::null();
  for (int8_t depth = 1; depth < 127; ++depth) {
    Move pv[128];
    size_t pvLen;
    const score_t score = stupidAlphaBetaSearch(board, depth, 0, -SCORE_INF, SCORE_INF,
                                                boardGetPsqScore(board), pv, pvLen, state);
    if (!isScoreValid(score)) {
      logError(SEARCH_JOB) << "Search returned invalid score " << score;
    }
    if (control_.isStopped()) {
      server_->finishSearch(bestMove);
      return;
    }
    server_->sendResult({static_cast<size_t>(depth), pv, pvLen, scoreToPositionCost(score),
                         SoFBotApi::PositionCostBound::Exact});
    bestMove = pv[0];
  }

  server_->finishSearch(bestMove);
  control_.stop();
}

}  // namespace SoFSearch::Private
