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

#ifndef SOF_SEARCH_PRIVATE_JOB_INCLUDED
#define SOF_SEARCH_PRIVATE_JOB_INCLUDED

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>

#include "core/move.h"
#include "eval/score.h"
#include "search/private/limits.h"

namespace SoFBotApi {
class Server;
}  // namespace SoFBotApi

namespace SoFSearch::Private {

class TranspositionTable;
struct Position;

// Shared data between jobs, which allows them to communicate with each other and with outer world
class JobCommunicator {
public:
  // Tells all the jobs that they must stop the search
  void stop();

  // If the search is timed out and needs to be stopped, then returns `true` and calls `stop()`.
  // Otherwise, returns `false`
  bool checkTimeout();

  // Waits until `stop()` is called or time period `time` is expired. If `stop()` was called before
  // or during waiting, returns `true`. Note that this function may sometimes return before `time`
  // is expired.
  template <class Rep, class Period>
  bool wait(const std::chrono::duration<Rep, Period> time) {
    std::unique_lock lock(stopLock_);
    if (isStopped()) {
      return true;
    }
    stopEvent_.wait_for(lock, time);
    return isStopped();
  }

  // Returns `true` if the jobs must stop the search
  inline bool isStopped() const { return stopped_.load(std::memory_order_acquire); }

  // Returns the depth on which the jobs must search now
  inline size_t depth() const { return depth_.load(std::memory_order_acquire); }

  // Returns the time point when the search was started
  inline std::chrono::steady_clock::time_point startTime() const { return startTime_; }

  // Returns search limits for the current search
  inline const SearchLimits &limits() const { return limits_; }

  // Resets the job into its default state. This function must not be called when jobs are running.
  inline void reset(const SearchLimits &limits) {
    depth_.store(1, std::memory_order_relaxed);
    stopped_.store(false, std::memory_order_relaxed);
    startTime_ = Clock::now();
    limits_ = limits;
  }

  // Indicates that the job has finished to search on depth `depth`. Returns `true` if it was the
  // first job to finish search on this depth, otherwise returns false.
  inline bool finishDepth(size_t depth) {
    return depth_.compare_exchange_strong(depth, depth + 1, std::memory_order_acq_rel);
  }

private:
  using Clock = std::chrono::steady_clock;

  std::atomic<size_t> depth_ = 1;
  std::atomic<size_t> stopped_ = false;
  Clock::time_point startTime_ = Clock::now();
  SearchLimits limits_ = SearchLimits::withInfiniteTime();

  std::condition_variable stopEvent_;
  std::mutex stopLock_;
};

// Type of job stats
enum class JobStat : size_t {
  Nodes,
  TtHits,
  TtExactHits,
  PvNodes,
  NonPvNodes,
  PvInternalNodes,
  NonPvInternalNodes,
  PPEdges,  // PV -> PV transitions
  PNEdges,  // PV -> non-PV transitions
  NNEdges,  // non-PV -> non-PV transitions
  Max       // Fake type to denote the total number of stats
};

// Number of job stats
constexpr size_t JOB_STAT_SZ = static_cast<size_t>(JobStat::Max);

// Job results and statistics. This class is thread-safe if there is no more than one writer thread.
// If two threads write concurrently, the data race occurs.
class JobResults {
public:
  // These functions return the job results. They can be called by reader threads.
  inline uint64_t get(const JobStat stat) const {
    return getRelaxed(stats_[static_cast<size_t>(stat)]);
  }

  inline size_t depth() const { return getRelaxed(depth_); }
  inline SoFCore::Move bestMove() const { return getRelaxed(bestMove_); }

  // These functions update the job results. They can be called by a single writer thread.
  inline void inc(const JobStat stat) {
    std::atomic<uint64_t> &value = stats_[static_cast<size_t>(stat)];
    const uint64_t newValue = value.load(std::memory_order_relaxed) + 1;
    value.store(newValue, std::memory_order_relaxed);
  }

  inline void setBestMove(const size_t depth, const SoFCore::Move move) {
    depth_.store(depth, std::memory_order_relaxed);
    bestMove_.store(move, std::memory_order_relaxed);
  }

private:
  template <typename T>
  inline static T getRelaxed(const std::atomic<T> &value) {
    return value.load(std::memory_order_relaxed);
  }

  std::atomic<uint64_t> stats_[JOB_STAT_SZ] = {};
  std::atomic<size_t> depth_ = 0;
  std::atomic<SoFCore::Move> bestMove_ = SoFCore::Move::null();
};

static_assert(std::atomic<uint64_t>::is_always_lock_free);
static_assert(std::atomic<size_t>::is_always_lock_free);
static_assert(std::atomic<SoFCore::Move>::is_always_lock_free);

// A class that represents a single search job.
class Job {
public:
  inline Job(JobCommunicator &communicator, TranspositionTable &tt, SoFBotApi::Server &server,
             SoFEval::ScoreEvaluator &evaluator, size_t id)
      : communicator_(communicator), tt_(tt), server_(server), evaluator_(evaluator), id_(id) {}

  // Returns the current results of the search job. The results are updated while the job is
  // running.
  inline const JobResults &results() const { return results_; }

  // Starts the search job. This function must be called exactly once.
  void run(const Position &position);

private:
  friend class Searcher;

  JobCommunicator &communicator_;
  TranspositionTable &tt_;
  SoFBotApi::Server &server_;
  SoFEval::ScoreEvaluator &evaluator_;
  size_t id_;
  JobResults results_;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_JOB_INCLUDED
