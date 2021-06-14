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

#include "search/private/job.h"

#include <algorithm>
#include <chrono>
#include <vector>

#include "bot_api/types.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "eval/evaluate.h"
#include "eval/score.h"
#include "search/private/consts.h"
#include "search/private/diagnostics.h"
#include "search/private/move_picker.h"
#include "search/private/util.h"
#include "util/misc.h"
#include "util/random.h"

namespace SoFSearch::Private {

using SoFBotApi::PositionCostBound;
using SoFCore::Board;
using SoFCore::Move;
using SoFCore::MovePersistence;
using SoFEval::adjustCheckmate;
using SoFEval::SCORE_CHECKMATE_THRESHOLD;
using SoFEval::SCORE_INF;
using SoFEval::score_t;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

#ifdef USE_SEARCH_DIAGNOSTICS
using SoFEval::isScoreCheckmate;
using SoFEval::isScoreValid;
#endif

using Evaluator = SoFEval::Evaluator<score_t>;

void JobCommunicator::stop() {
  size_t tmp = 0;
  if (!stopped_.compare_exchange_strong(tmp, 1, std::memory_order_release)) {
    return;
  }
  // Lock and unlock `stopLock_` to ensure that we are not checking for `isStopped()` in
  // `this->wait()` now. If we remove lock/unlock from here, the following may happen:
  // - `this->wait()` checks for `isStopped()`, which returns `false`
  // - we change `stopped_` to `1` and notify all the waiting threads
  // - `this->wait()` goes to `stopEvent_.wait()` and doesn't wake, since the thread didn't started
  // waiting when nofitication arrived
  stopLock_.lock();
  stopLock_.unlock();
  stopEvent_.notify_all();
}

class Searcher {
public:
  enum class NodeKind { Root, Pv, Simple };

  inline constexpr static bool isNodeKindPv(const NodeKind kind) {
    return kind == NodeKind::Root || kind == NodeKind::Pv;
  }

  inline Searcher(Job &job, Board &board, const SearchLimits &limits, RepetitionTable &repetitions)
      : board_(board),
        tt_(job.table_),
        comm_(job.communicator_),
        results_(job.results_),
        repetitions_(repetitions),
        limits_(limits),
        jobId_(job.id_),
        startTime_(steady_clock::now()) {}

  inline score_t run(const size_t depth, Move &bestMove) {
    depth_ = depth;
    const score_t score =
        search<NodeKind::Root>(static_cast<int32_t>(depth), 0, -SCORE_INF, SCORE_INF,
                               Evaluator::Tag::from(board_), FLAGS_DEFAULT);
    DGN_ASSERT(isScoreValid(score));
    bestMove = stack_[0].bestMove;
    return score;
  }

private:
  struct Frame {
    KillerLine killers;  // Must be preserved across recursive calls
    Move bestMove = Move::null();
  };

  // Search flags type
  using flags_t = uint64_t;

  // Search flags
  static constexpr flags_t FLAG_CAPTURE = 1;

  // Predefined search flag sets
  static constexpr flags_t FLAGS_DEFAULT = 0;

  inline bool mustStop() const {
    if (comm_.isStopped()) {
      return true;
    }
    ++counter_;
    if (!(counter_ & 4095)) {
      if (limits_.time != TIME_UNLIMITED && steady_clock::now() - startTime_ >= limits_.time) {
        comm_.stop();
        return true;
      }
    }
    return comm_.depth() != depth_;
  }

  template <NodeKind Node>
  inline score_t search(const size_t depth, const size_t idepth, const score_t alpha,
                        const score_t beta, const Evaluator::Tag tag, const flags_t flags) {
    tt_.prefetch(board_.hash);
    if (!repetitions_.insert(board_.hash)) {
      return 0;
    }
    const score_t score = doSearch<Node>(depth, idepth, alpha, beta, tag, flags);
    DGN_ASSERT(score <= alpha || score >= beta ||
               isScoreValid(adjustCheckmate(score, -static_cast<int16_t>(idepth))));
    repetitions_.erase(board_.hash);
    return score;
  }

  template <NodeKind Node>
  score_t doSearch(int32_t depth, size_t idepth, score_t alpha, score_t beta, Evaluator::Tag tag,
                   flags_t flags);

  score_t quiescenseSearch(score_t alpha, score_t beta, Evaluator::Tag tag);

  Board &board_;
  TranspositionTable &tt_;
  JobCommunicator &comm_;
  JobResults &results_;
  RepetitionTable &repetitions_;
  SearchLimits limits_;
  Evaluator evaluator_;
  size_t jobId_;

  Frame stack_[MAX_DEPTH + 10];
  HistoryTable history_;
  size_t depth_ = 0;
  mutable size_t counter_ = 0;
  steady_clock::time_point startTime_;
};

class RootNodeMovePicker {
public:
  RootNodeMovePicker(MovePicker picker, const size_t jobId) {
    for (Move move = picker.next(); move != Move::invalid(); move = picker.next()) {
      if (move == Move::null()) {
        continue;
      }
      moves_[moveCount_++] = move;
    }
    if (jobId == 0) {
      return;
    }
    if (jobId < moveCount_) {
      std::reverse(moves_, moves_ + jobId);
    } else {
      SoFUtil::randomShuffle(moves_, moves_ + moveCount_);
    }
  }

  inline Move next() {
    if (pos_ == moveCount_) {
      return Move::invalid();
    }
    return moves_[pos_++];
  }

private:
  Move moves_[SoFCore::BUFSZ_MOVES];
  size_t moveCount_ = 0;
  size_t pos_ = 0;
};

template <Searcher::NodeKind Kind>
struct MovePickerFactory {
  template <typename... Args>
  inline static MovePicker create([[maybe_unused]] const size_t jobId, Args &&...args) {
    return MovePicker(std::forward<Args>(args)...);
  }
};

template <>
struct MovePickerFactory<Searcher::NodeKind::Root> {
  template <typename... Args>
  inline static RootNodeMovePicker create(const size_t jobId, Args &&...args) {
    return RootNodeMovePicker(MovePicker(std::forward<Args>(args)...), jobId);
  }
};

#ifdef USE_SEARCH_DIAGNOSTICS
// Check that the moves are not repeated
class DgnMoveRepeatChecker {
public:
  inline void add(const Move move) {
    for (size_t i = 0; i < count_; ++i) {
      if (SOF_UNLIKELY(moves_[i] == move)) {
        SoFUtil::panic("Move " + moveToStr(move) + " is repeated twice!");
      }
    }
    moves_[count_++] = move;
  }

private:
  size_t count_ = 0;
  Move moves_[SoFCore::BUFSZ_MOVES];
};
#endif

score_t Searcher::quiescenseSearch(score_t alpha, const score_t beta, const Evaluator::Tag tag) {
  if (isBoardDrawInsufficientMaterial(board_)) {
    return 0;
  }

  {
    const score_t score = evaluator_.evalForCur(board_, tag);
    DIAGNOSTIC({
      if (alpha < score && score < beta) {
        DGN_ASSERT(isScoreValid(score));
        DGN_ASSERT(!isScoreCheckmate(score));
      }
    });
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
      return beta;
    }
  }

  DIAGNOSTIC(DgnMoveRepeatChecker dgnMoves;)
  QuiescenseMovePicker picker(board_);
  for (Move move = picker.next(); move != Move::invalid(); move = picker.next()) {
    if (move == Move::null()) {
      continue;
    }
    DIAGNOSTIC(dgnMoves.add(move);)
    const Evaluator::Tag newTag = tag.updated(board_, move);
    const MovePersistence persistence = moveMake(board_, move);
    DGN_ASSERT(newTag.isValid(board_));
    if (!isMoveLegal(board_)) {
      moveUnmake(board_, move, persistence);
      continue;
    }
    results_.inc(JobStat::Nodes);
    const score_t score = -quiescenseSearch(-beta, -alpha, newTag);
    DIAGNOSTIC({
      if (alpha < score && score < beta) {
        DGN_ASSERT(isScoreValid(score));
        DGN_ASSERT(!isScoreCheckmate(score));
      }
    });
    moveUnmake(board_, move, persistence);
    if (mustStop()) {
      return 0;
    }
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
      return beta;
    }
  }

  return alpha;
}

template <Searcher::NodeKind Node>
score_t Searcher::doSearch(const int32_t depth, const size_t idepth, score_t alpha,
                           const score_t beta, const Evaluator::Tag tag,
                           [[maybe_unused]] const flags_t flags) {
  const score_t origAlpha = alpha;
  const score_t origBeta = beta;
  Frame &frame = stack_[idepth];
  frame.bestMove = Move::null();

  // Check for draw
  if constexpr (Node != NodeKind::Root) {
    if (board_.moveCounter >= 100) {
      return 0;
    }
    if (isBoardDrawInsufficientMaterial(board_)) {
      return 0;
    }
  }

  // Run quiescence search in leaf node
  if (depth <= 0) {
    if (alpha >= SCORE_CHECKMATE_THRESHOLD) {
      return alpha;
    }
    if (beta <= -SCORE_CHECKMATE_THRESHOLD) {
      return beta;
    }
    return quiescenseSearch(alpha, beta, tag);
  }

  auto ttStore = [&](score_t score) {
    PositionCostBound bound = PositionCostBound::Exact;
    if (score <= origAlpha) {
      score = origAlpha;
      bound = PositionCostBound::Upperbound;
    }
    if (score >= origBeta) {
      score = origBeta;
      bound = PositionCostBound::Lowerbound;
    }
    score = adjustCheckmate(score, -static_cast<int16_t>(idepth));
    DGN_ASSERT(bound != PositionCostBound::Exact || isScoreValid(score));
    tt_.store(board_.hash,
              TranspositionTable::Data(frame.bestMove, score, depth, bound, isNodeKindPv(Node)));
  };

  // Probe the transposition table
  Move hashMove = Move::null();
  if (const TranspositionTable::Data data = tt_.load(board_.hash); data.isValid()) {
    results_.inc(JobStat::TtHits);
    hashMove = data.move();
    if (Node != NodeKind::Root && data.depth() >= depth && board_.moveCounter < 90) {
      const score_t score = adjustCheckmate(data.score(), static_cast<int16_t>(idepth));
      switch (data.bound()) {
        case PositionCostBound::Exact: {
          frame.bestMove = hashMove;
          results_.inc(JobStat::TtExactHits);
          // Refresh the hash entry, as it may come from older epoch
          tt_.refresh(board_.hash, data);
          return score;
        }
        case PositionCostBound::Lowerbound: {
          if (score >= beta) {
            return beta;
          }
          break;
        }
        case PositionCostBound::Upperbound: {
          if (alpha >= score) {
            return alpha;
          }
          break;
        }
      }
    }
  }

  // Iterate over the moves in the sorted order
  auto picker = MovePickerFactory<Node>::create(jobId_, board_, hashMove, frame.killers, history_);
  bool hasMove = false;
  DIAGNOSTIC(DgnMoveRepeatChecker dgnMoves;)
  for (Move move = picker.next(); move != Move::invalid(); move = picker.next()) {
    if (move == Move::null()) {
      continue;
    }
    DIAGNOSTIC(dgnMoves.add(move);)
    const Evaluator::Tag newTag = tag.updated(board_, move);
    const MovePersistence persistence = moveMake(board_, move);
    DGN_ASSERT(newTag.isValid(board_));
    if (!isMoveLegal(board_)) {
      moveUnmake(board_, move, persistence);
      continue;
    }
    results_.inc(JobStat::Nodes);
    const flags_t newFlags = isMoveCapture(board_, move) ? FLAG_CAPTURE : 0;
    if (hasMove && beta != alpha + 1 &&
        -search<NodeKind::Simple>(depth - 1, idepth + 1, -alpha - 1, -alpha, newTag, newFlags) <=
            alpha) {
      moveUnmake(board_, move, persistence);
      if (mustStop()) {
        return 0;
      }
      continue;
    }
    hasMove = true;
    constexpr NodeKind newNode = (Node == NodeKind::Simple ? NodeKind::Simple : NodeKind::Pv);
    const score_t score = -search<newNode>(depth - 1, idepth + 1, -beta, -alpha, newTag, newFlags);
    moveUnmake(board_, move, persistence);
    if (mustStop()) {
      return 0;
    }
    if (score > alpha) {
      alpha = score;
      frame.bestMove = move;
    }
    if (alpha >= beta) {
      if constexpr (Node != NodeKind::Root) {
        if (picker.stage() >= MovePickerStage::Killer) {
          frame.killers.add(move);
          history_[move] += depth * depth;
        }
      }
      ttStore(beta);
      return beta;
    }
  }

  // Detect checkmate and stalemate
  if (!hasMove) {
    return isCheck(board_) ? SoFEval::scoreCheckmateLose(static_cast<int16_t>(idepth)) : 0;
  }

  // End of search
  ttStore(alpha);
  return alpha;
}

std::vector<Move> unwindPv(Board board, const Move bestMove, TranspositionTable &tt) {
  RepetitionTable repetitions;
  repetitions.insert(board.hash);
  std::vector<Move> pv{bestMove};
  moveMake(board, bestMove);
  repetitions.insert(board.hash);
  for (;;) {
    TranspositionTable::Data data = tt.load(board.hash);
    if (!data.isValid() || data.move() == Move::null() ||
        data.bound() != PositionCostBound::Exact) {
      break;
    }
    // Refresh the hash entry, as it may come from older epoch
    tt.refresh(board.hash, data);
    const Move move = data.move();
    moveMake(board, move);
    if (!repetitions.insert(board.hash)) {
      break;
    }
    pv.push_back(move);
  }
  return pv;
}

void Job::run(const Position &position, const SearchLimits &limits) {
  // Apply moves and fill repetition tables
  Board board = position.first;
  RepetitionTable singleRepeat;
  RepetitionTable doubleRepeat;
  for (const Move move : position.moves) {
    if (!singleRepeat.insert(board.hash)) {
      doubleRepeat.insert(board.hash);
    }
    moveMake(board, move);
  }

  // Perform iterative deepening
  Searcher searcher(*this, board, limits, doubleRepeat);
  const size_t maxDepth = std::min(limits.depth, MAX_DEPTH);
  for (size_t depth = 1; depth <= maxDepth; ++depth) {
    Move bestMove = Move::null();
    const score_t score = searcher.run(depth, bestMove);
    if (communicator_.isStopped()) {
      return;
    }
    if (communicator_.finishDepth(depth)) {
      DGN_ASSERT(bestMove != Move::null());
      results_.setBestMove(depth, bestMove);
      std::vector<Move> pv = unwindPv(board, bestMove, table_);
      server_.sendResult({depth, pv.data(), pv.size(), SoFEval::scoreToPositionCost(score),
                          PositionCostBound::Exact});
    }
  }

  communicator_.stop();
}

}  // namespace SoFSearch::Private
