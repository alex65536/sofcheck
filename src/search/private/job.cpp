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

#include "search/private/job.h"

#include <algorithm>
#include <chrono>
#include <utility>
#include <vector>

#include "bot_api/server.h"
#include "bot_api/types.h"
#include "core/board.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "eval/evaluate.h"
#include "eval/score.h"
#include "search/private/consts.h"
#include "search/private/diagnostics.h"
#include "search/private/limits.h"
#include "search/private/move_picker.h"
#include "search/private/transposition_table.h"
#include "search/private/types.h"
#include "search/private/util.h"
#include "util/misc.h"
#include "util/no_copy_move.h"
#include "util/operators.h"
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

// RAII wrapper that unmakes the move on destruction
class MoveMakeGuard : public SoFUtil::NoCopyMove {
public:
  // Makes a move `move` on board `board`. `tag` must be strictly equal to `Tag::from(b)`, i. e.
  // `tag.isValid(board)` must hold
  MoveMakeGuard(Board &board, const Move move, const Evaluator::Tag &tag)
      : board_(board),
        tag_(tag.updated(board, move)),
        persistence_(moveMake(board, move)),
        move_(move),
        active_(true) {
    DGN_ASSERT(tag_.isValid(board_));
  }

  // Unmakes the move, releasing the guard
  void release() {
    if (active_) {
      active_ = false;
      moveUnmake(board_, move_, persistence_);
    }
  }

  // Returns the tag equal to `Tag::from(board_)` at the time the guard was created
  Evaluator::Tag tag() const {
    DGN_ASSERT(active_);
    return tag_;
  }

  ~MoveMakeGuard() { release(); }

private:
  Board &board_;
  Evaluator::Tag tag_;
  MovePersistence persistence_;
  Move move_;
  bool active_;
};

class Searcher {
public:
  enum class NodeKind { Root, Pv, Simple };

  // Search flags
  enum class Flags : uint64_t {
    None = 0,
    // The last move was capture
    Capture = 1,
    // We are inside the null move search
    NullMove = 2,
    // Null move reduction was applied in this branch
    NullMoveReduction = 4,
    // Late move reduction was applied in this branch
    LateMoveReduction = 8,

    // Below there are predefined flag sets

    // All flags are set
    All = 15,
    // Flags set by default
    Default = 0,
    // Flags that are preserved when doing a recursive call
    Inherit = NullMove | NullMoveReduction | LateMoveReduction,
    // Each of these flags disables null move heuristics
    NullMoveDisable = NullMove | NullMoveReduction | Capture
  };

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
                               Evaluator::Tag::from(board_), Flags::Default);
    DGN_ASSERT(isScoreValid(score));
    bestMove = stack_[0].bestMove;
    return score;
  }

private:
  struct Frame {
    KillerLine killers;  // Must be preserved across recursive calls
    Move bestMove = Move::null();
  };

  inline constexpr static bool isNodeKindPv(const NodeKind kind) {
    return kind == NodeKind::Root || kind == NodeKind::Pv;
  }

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
                        const score_t beta, const Evaluator::Tag tag, const Flags flags) {
    tt_.prefetch(board_.hash);
    if (!repetitions_.insert(board_.hash)) {
      return 0;
    }
    DIAGNOSTIC(const SoFCore::board_hash_t savedHash = board_.hash);
    const score_t score =
        doSearch<Node>(static_cast<int32_t>(depth), idepth, alpha, beta, tag, flags);
    DGN_ASSERT(score <= alpha || score >= beta ||
               isScoreValid(adjustCheckmate(score, -static_cast<int16_t>(idepth))));
    DGN_ASSERT(board_.hash == savedHash);
    repetitions_.erase(board_.hash);
    return score;
  }

  template <NodeKind Node>
  score_t doSearch(int32_t depth, size_t idepth, score_t alpha, score_t beta, Evaluator::Tag tag,
                   Flags flags);

  score_t quiescenseSearch(score_t alpha, score_t beta, Evaluator::Tag tag);

  Board &board_;
  TranspositionTable &tt_;
  JobCommunicator &comm_;
  JobResults &results_;
  RepetitionTable &repetitions_;
  SearchLimits limits_;
  Evaluator evaluator_;
  size_t jobId_;

  Frame stack_[MAX_STACK_DEPTH];
  HistoryTable history_;
  size_t depth_ = 0;
  mutable size_t counter_ = 0;
  steady_clock::time_point startTime_;
};

SOF_ENUM_BITWISE(Searcher::Flags, uint64_t)
SOF_ENUM_EQUAL(Searcher::Flags, uint64_t)

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

  const score_t evalScore = evaluator_.evalForCur(board_, tag);
  DIAGNOSTIC({
    if (alpha < evalScore && evalScore < beta) {
      DGN_ASSERT(isScoreValid(evalScore));
      DGN_ASSERT(!isScoreCheckmate(evalScore));
    }
  });
  alpha = std::max(alpha, evalScore);
  if (alpha >= beta) {
    return beta;
  }

  DIAGNOSTIC(DgnMoveRepeatChecker dgnMoves;)
  QuiescenseMovePicker picker(board_);
  for (Move move = picker.next(); move != Move::invalid(); move = picker.next()) {
    if (move == Move::null()) {
      continue;
    }
    DIAGNOSTIC(dgnMoves.add(move);)
    MoveMakeGuard guard(board_, move, tag);
    if (!isMoveLegal(board_)) {
      continue;
    }
    results_.inc(JobStat::Nodes);
    const score_t score = -quiescenseSearch(-beta, -alpha, guard.tag());
    DIAGNOSTIC({
      if (alpha < score && score < beta) {
        DGN_ASSERT(isScoreValid(score));
        DGN_ASSERT(!isScoreCheckmate(score));
      }
    });
    if (mustStop()) {
      return 0;
    }
    guard.release();
    alpha = std::max(alpha, score);
    if (alpha >= beta) {
      return beta;
    }
  }

  return alpha;
}

template <Searcher::NodeKind Node>
score_t Searcher::doSearch(int32_t depth, const size_t idepth, score_t alpha, const score_t beta,
                           const Evaluator::Tag tag, Flags flags) {
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

  // Run quiescence search. We do this in two cases: either the depth is less than zero, meaning
  // that it's the leaf node, or that we will overflow the `stack_` array if we continue the
  // recursion. The second case is pretty unlikely, but is necessary to improve the engine
  // robustness
  if (depth <= 0 || idepth + 1 == MAX_STACK_DEPTH) {
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

  const bool isInCheck = isCheck(board_);
  const bool isMateBounds =
      alpha <= -SCORE_CHECKMATE_THRESHOLD || beta >= SCORE_CHECKMATE_THRESHOLD;

  // Futility pruning
  if (!isNodeKindPv(Node) && depth <= Futility::MAX_DEPTH && !isInCheck && !isMateBounds) {
    const score_t threshold = beta + Futility::MARGIN;
    if (evaluator_.evalForCur(board_, tag) >= threshold) {
      return beta;
    }
  }

  // Null move heuristics. Currently, it's implemented as null move reduction, as this variant is
  // less prone to zugzwang and not significantly slower than just pruning the branch
  const bool canNullMove = !isNodeKindPv(Node) && depth >= NullMove::MIN_DEPTH && !isInCheck &&
                           !isMateBounds && (flags & Flags::NullMoveDisable) == Flags::None;
  if (canNullMove) {
    MoveMakeGuard guard(board_, Move::null(), tag);
    DGN_ASSERT(isMoveLegal(board_));
    results_.inc(JobStat::Nodes);
    const Flags newFlags = (flags & Flags::Inherit) | Flags::NullMove;
    const score_t score = -search<NodeKind::Simple>(depth - NullMove::DEPTH_DEC, idepth + 1, -beta,
                                                    -beta + 1, guard.tag(), newFlags);
    if (mustStop()) {
      return 0;
    }
    guard.release();
    if (score >= beta) {
      depth -= NullMove::REDUCTION_DEC;
      flags |= Flags::NullMoveReduction;
      DGN_ASSERT(depth > 0);
    }
  }

  // Iterate over the moves in the sorted order
  auto picker = MovePickerFactory<Node>::create(jobId_, board_, hashMove, frame.killers, history_);
  bool hasMove = false;
  size_t numHistoryMoves = 0;
  DIAGNOSTIC(DgnMoveRepeatChecker dgnMoves;)
  for (Move move = picker.next(); move != Move::invalid(); move = picker.next()) {
    if (move == Move::null()) {
      continue;
    }
    DIAGNOSTIC(dgnMoves.add(move);)
    const bool isCapture = isMoveCapture(board_, move);
    MoveMakeGuard guard(board_, move, tag);
    if (!isMoveLegal(board_)) {
      continue;
    }
    if constexpr (Node != NodeKind::Root) {
      if (picker.stage() == MovePickerStage::History) {
        ++numHistoryMoves;
      }
    }
    results_.inc(JobStat::Nodes);
    const Flags newFlags = (flags & Flags::Inherit) | (isCapture ? Flags::Capture : Flags::None);

    // Late move reduction (LMR)
    if constexpr (Node != NodeKind::Root) {
      const bool lmrEnabled = hasMove && !isNodeKindPv(Node) && depth >= LateMove::MIN_DEPTH &&
                              picker.stage() == MovePickerStage::History &&
                              numHistoryMoves > LateMove::MOVES_NO_REDUCE && !isCheck(board_);
      if (lmrEnabled) {
        const score_t score =
            -search<NodeKind::Simple>(depth - 1 - LateMove::REDUCE_DEPTH, idepth + 1, -alpha - 1,
                                      -alpha, guard.tag(), newFlags | Flags::LateMoveReduction);
        if (mustStop()) {
          return 0;
        }
        if (score <= alpha) {
          continue;
        }
      }
    }

    if (hasMove && beta != alpha + 1) {
      const score_t score = -search<NodeKind::Simple>(depth - 1, idepth + 1, -alpha - 1, -alpha,
                                                      guard.tag(), newFlags);
      if (mustStop()) {
        return 0;
      }
      if (score <= alpha) {
        continue;
      }
    }
    hasMove = true;
    constexpr NodeKind newNode = (Node == NodeKind::Simple ? NodeKind::Simple : NodeKind::Pv);
    const score_t score =
        -search<newNode>(depth - 1, idepth + 1, -beta, -alpha, guard.tag(), newFlags);
    if (mustStop()) {
      return 0;
    }
    guard.release();
    if (score > alpha) {
      alpha = score;
      frame.bestMove = move;
    }
    if (alpha >= beta) {
      if constexpr (Node != NodeKind::Root) {
        if (picker.stage() >= MovePickerStage::Killer) {
          frame.killers.add(move);
          // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
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
