#include "search/private/job.h"

#include <algorithm>
#include <chrono>
#include <vector>

#include "bot_api/types.h"
#include "core/movegen.h"
#include "search/private/evaluate.h"
#include "search/private/move_picker.h"
#include "search/private/score.h"
#include "search/private/util.h"
#include "util/random.h"

namespace SoFSearch::Private {

using SoFBotApi::PositionCostBound;
using SoFCore::Board;
using SoFCore::Color;
using SoFCore::Move;
using SoFCore::MovePersistence;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

void JobCommunicator::stop() {
  size_t tmp = 0;
  if (!stopped_.compare_exchange_strong(tmp, 1, std::memory_order_seq_cst)) {
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

constexpr size_t MAX_DEPTH = 255;

class Searcher {
public:
  enum class NodeKind { Root, Pv, Simple };

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
        search<NodeKind::Root>(depth, 0, -SCORE_INF, SCORE_INF, boardGetPsqScore(board_));
    bestMove = stack_[0].bestMove;
    return score;
  }

private:
  struct Frame {
    KillerLine killers;  // Must be preserved across recursive calls
    Move bestMove = Move::null();
  };

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
                        const score_t beta, const score_pair_t psq) {
    tt_.prefetch(board_.hash);
    if (!repetitions_.insert(board_.hash)) {
      return 0;
    }
    const score_t score = doSearch<Node>(depth, idepth, alpha, beta, psq);
    repetitions_.erase(board_.hash);
    return score;
  }

  template <NodeKind Node>
  score_t doSearch(size_t depth, size_t idepth, score_t alpha, score_t beta, score_pair_t psq);

  score_t quiescenseSearch(score_t alpha, score_t beta, score_pair_t psq);

  Board &board_;
  TranspositionTable &tt_;
  JobCommunicator &comm_;
  JobResults &results_;
  RepetitionTable &repetitions_;
  SearchLimits limits_;
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
  inline static MovePicker create([[maybe_unused]] const size_t jobId, Args &&... args) {
    return MovePicker(std::forward<Args>(args)...);
  }
};

template <>
struct MovePickerFactory<Searcher::NodeKind::Root> {
  template <typename... Args>
  inline static RootNodeMovePicker create(const size_t jobId, Args &&... args) {
    return RootNodeMovePicker(MovePicker(std::forward<Args>(args)...), jobId);
  }
};

score_t Searcher::quiescenseSearch(score_t alpha, const score_t beta, const score_pair_t psq) {
  score_t score = evaluate(board_, psq, alpha, beta);
  alpha = std::max(alpha, score);
  if (alpha >= beta) {
    return beta;
  }

  QuiescenseMovePicker picker(board_);
  for (Move move = picker.next(); move != Move::invalid(); move = picker.next()) {
    if (move == Move::null()) {
      continue;
    }
    const score_pair_t newPsq = boardUpdatePsqScore(board_, move, psq);
    const MovePersistence persistence = moveMake(board_, move);
    if (!isMoveLegal(board_)) {
      moveUnmake(board_, move, persistence);
      continue;
    }
    results_.inc(JobStat::Nodes);
    const score_t score = -quiescenseSearch(-beta, -alpha, newPsq);
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
score_t Searcher::doSearch(const size_t depth, const size_t idepth, score_t alpha,
                           const score_t beta, const score_pair_t psq) {
  const score_t origAlpha = alpha;
  const score_t origBeta = beta;
  Frame &frame = stack_[idepth];
  frame.bestMove = Move::null();

  // 0. Check for draw by 50 moves
  if (board_.moveCounter >= 100) {
    return 0;
  }

  // 1. Run quiescence search in leaf node
  if (depth == 0) {
    return quiescenseSearch(alpha, beta, psq);
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
    tt_.store(board_.hash, TranspositionTable::Data(frame.bestMove, score, depth, bound));
  };

  // 2. Probe the transposition table
  Move hashMove = Move::null();
  if (const TranspositionTable::Data data = tt_.load(board_.hash); data.isValid()) {
    results_.inc(JobStat::TtHits);
    hashMove = data.move();
    if (Node == NodeKind::Simple && data.depth() >= depth && board_.moveCounter < 90) {
      const score_t score = adjustCheckmate(data.score(), idepth);
      switch (data.bound()) {
        case PositionCostBound::Exact: {
          frame.bestMove = hashMove;
          // Refresh the hash entry, as it may come from older epoch
          tt_.store(board_.hash, data);
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

  // 3. Iterate over the moves in the sorted order
  auto picker = MovePickerFactory<Node>::create(jobId_, board_, hashMove, frame.killers, history_);
  bool hasMove = false;
  for (Move move = picker.next(); move != Move::invalid(); move = picker.next()) {
    if (move == Move::null()) {
      continue;
    }
    const score_pair_t newPsq = boardUpdatePsqScore(board_, move, psq);
    const MovePersistence persistence = moveMake(board_, move);
    if (!isMoveLegal(board_)) {
      moveUnmake(board_, move, persistence);
      continue;
    }
    results_.inc(JobStat::Nodes);
    if (hasMove &&
        -search<NodeKind::Simple>(depth - 1, idepth + 1, -alpha - 1, -alpha, newPsq) <= alpha) {
      moveUnmake(board_, move, persistence);
      if (mustStop()) {
        return 0;
      }
      continue;
    }
    hasMove = true;
    constexpr NodeKind newNode = (Node == NodeKind::Simple ? NodeKind::Simple : NodeKind::Pv);
    const score_t score = -search<newNode>(depth - 1, idepth + 1, -beta, -alpha, newPsq);
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

  // 4. Detect checkmate and stalemate
  if (!hasMove) {
    return isCheck(board_) ? scoreCheckmateLose(idepth) : 0;
  }

  // 5. End of search
  ttStore(alpha);
  return alpha;
}

std::vector<Move> unwindPv(Board board, const Move bestMove, const TranspositionTable &tt) {
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
      // FIXME: check that best move is not null and score is valid
      results_.setBestMove(depth, bestMove);
      std::vector<Move> pv = unwindPv(board, bestMove, table_);
      server_.sendResult(
          {depth, pv.data(), pv.size(), scoreToPositionCost(score), PositionCostBound::Exact});
    }
  }

  communicator_.stop();
}

}  // namespace SoFSearch::Private
