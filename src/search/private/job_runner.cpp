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

#include "search/private/job_runner.h"

#include <chrono>
#include <cstdint>
#include <deque>
#include <sstream>
#include <string>

#include "bot_api/server.h"
#include "core/board.h"
#include "core/move.h"
#include "core/movegen.h"
#include "eval/evaluate.h"
#include "search/private/limits.h"
#include "search/private/types.h"
#include "util/defer.h"
#include "util/logging.h"
#include "util/math.h"
#include "util/no_copy_move.h"
#include "util/random.h"

namespace SoFSearch::Private {

using namespace std::chrono_literals;
using namespace SoFUtil::Logging;

using SoFCore::Board;
using SoFCore::Move;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;

// Type of log entry
constexpr const char *JOB_RUNNER = "JobRunner";

class Stats {
public:
  inline uint64_t get(const JobStat stat) const { return stats_[static_cast<size_t>(stat)]; }
  inline double getDouble(const JobStat stat) const { return static_cast<double>(get(stat)); }

  inline uint64_t nodes() const { return get(JobStat::Nodes); }
  inline uint64_t pvNodes() const { return get(JobStat::PvNodes); }
  inline uint64_t nonPvNodes() const { return get(JobStat::NonPvNodes); }
  inline uint64_t otherNodes() const { return nodes() - pvNodes() - nonPvNodes(); }

  inline double pvBranchFactor() const {
    return static_cast<double>(get(JobStat::PPEdges) + get(JobStat::PNEdges)) /
           getDouble(JobStat::PvInternalNodes);
  }

  inline double nonPvBranchFactor() const {
    return getDouble(JobStat::NNEdges) / getDouble(JobStat::NonPvInternalNodes);
  }

  inline double branchFactor() const {
    return static_cast<double>(get(JobStat::PPEdges) + get(JobStat::PNEdges) +
                               get(JobStat::NNEdges)) /
           static_cast<double>(get(JobStat::PvInternalNodes) + get(JobStat::NonPvInternalNodes));
  }

  inline void add(const JobStats &jobStats) {
    for (size_t i = 0; i < JOB_STAT_SZ; ++i) {
      stats_[i] += jobStats.get(static_cast<JobStat>(i));
    }
  }

private:
  uint64_t stats_[JOB_STAT_SZ] = {};
};

static Move pickRandomMove(Board board) {
  Move moves[SoFCore::BUFSZ_MOVES];
  const size_t count = genAllMoves(board, moves);
  SoFUtil::randomShuffle(moves, moves + count);
  for (size_t i = 0; i < count; ++i) {
    const Move move = moves[i];
    const SoFCore::MovePersistence persistence = moveMake(board, move);
    if (!isMoveLegal(board)) {
      moveUnmake(board, move, persistence);
      continue;
    }
    moveUnmake(board, move, persistence);
    return move;
  }
  return Move::null();
}

class JobRunner::MainThread : public SoFUtil::NoCopyMove {
public:
  explicit MainThread(JobRunner &p, const Position &position, const size_t jobCount)
      : p_(p),
        position_(position),
        jobCount_(jobCount),
        comm_(p_.comm_),
        server_(p_.server_),
        startTime_(comm_.startTime()),
        limits_(comm_.limits()) {}

  void run() {
    disableReconfiguration();
    SOF_DEFER({ enableReconfiguration(); });
    createJobsAndThreads();
    runMainLoop();
    joinThreads();
    finishSearch();
  }

private:
  using Event = JobCommunicator::Event;

  static constexpr microseconds STATS_UPDATE_INTERVAL = 3s;
  static constexpr microseconds THREAD_TICK_INTERVAL = 30ms;

  microseconds calcSleepTime() const {
    if (limits_.time == TIME_UNLIMITED) {
      return THREAD_TICK_INTERVAL;
    }
    const auto timePassed = timeElapsed(steady_clock::now());
    const microseconds timeLeft = limits_.time - timePassed;
    const auto delay = std::min(timeLeft + 100us, THREAD_TICK_INTERVAL);
    return std::max(delay, 100us);
  }

  // Disables job runner reconfiguration. Must be called before the jobs are created
  void disableReconfiguration() {
    std::unique_lock lock(p_.applyConfigLock_);
    p_.canApplyConfig_ = false;
  }

  // Re-enables job runner reconfiguration and performs delayed configuration requests. Must be
  // called after the search is done
  void enableReconfiguration() {
    std::unique_lock lock(p_.applyConfigLock_);
    p_.canApplyConfig_ = true;
    p_.tryApplyConfigUnlocked();
  }

  void createJobsAndThreads() {
    SOF_ASSERT(p_.evaluators_.size() == jobCount_);
    for (size_t i = 0; i < jobCount_; ++i) {
      jobs_.emplace_back(comm_, p_.tt_, p_.evaluators_[i], i);
    }
    for (size_t i = 0; i < jobCount_; ++i) {
      threads_.emplace_back([&job = jobs_[i], this]() { job.run(position_); });
    }
  }

  void updateStats() {
    stats_ = Stats();
    for (const Job &job : jobs_) {
      stats_.add(job.stats());
    }
  }

  void extractLines() {
    auto lines = comm_.extractLines();
    // Normally, the extracted lines will be sorted by depth, but the jobs add them in a quite racy
    // manner, so they may appear in any order. Thus, we sort them to reduce the chaos a little.
    std::sort(lines.begin(), lines.end(),
              [&](const auto &a, const auto &b) { return a.depth < b.depth; });
    for (const auto &line : lines) {
      server_.sendResult(line, stats_.nodes());
      if (line.depth > bestDepth_ && !line.pv.empty()) {
        bestDepth_ = line.depth;
        bestMove_ = line.pv[0];
      }
    }
  }

  microseconds timeElapsed(const steady_clock::time_point &now) const {
    return duration_cast<microseconds>(now - startTime_);
  }

  bool mustStop(const steady_clock::time_point &now) const {
    return stats_.nodes() > limits_.nodes ||
           (limits_.time != TIME_UNLIMITED && timeElapsed(now) > limits_.time);
  }

  void printStats() {
    server_.sendNodeCount(stats_.nodes());
    server_.sendHashHits(stats_.get(JobStat::TtHits));

    if (p_.isDebugMode()) {
      std::ostringstream nodeStream;
      nodeStream << "Node counts: P = " << stats_.pvNodes() << " N = " << stats_.nonPvNodes()
                 << " O = " << stats_.otherNodes()
                 << " PI = " << stats_.get(JobStat::PvInternalNodes)
                 << " NI = " << stats_.get(JobStat::NonPvInternalNodes);

      std::ostringstream edgeStream;
      edgeStream.precision(3);
      edgeStream.flags(edgeStream.flags() | std::ostream::fixed);
      edgeStream << "Edge counts: PP = " << stats_.get(JobStat::PPEdges)
                 << " PN = " << stats_.get(JobStat::PNEdges)
                 << " NN = " << stats_.get(JobStat::NNEdges)
                 << " PBranch = " << stats_.pvBranchFactor()
                 << " NBranch = " << stats_.nonPvBranchFactor()
                 << " Branch = " << stats_.branchFactor();

      server_.sendString("Hash exact hits: " + std::to_string(stats_.get(JobStat::TtExactHits)));
      server_.sendString(nodeStream.str());
      server_.sendString(edgeStream.str());
    }
  }

  void runMainLoop() {
    auto statsLastUpdatedTime = startTime_;
    for (;;) {
      auto event = comm_.waitForEvent(calcSleepTime());
      if (event == Event::Stopped) {
        break;
      }
      const auto now = steady_clock::now();
      updateStats();
      if (event == Event::NewLines) {
        extractLines();
      }
      if (mustStop(now)) {
        comm_.stop();
      }
      if (now >= statsLastUpdatedTime + STATS_UPDATE_INTERVAL) {
        printStats();
        while (now >= statsLastUpdatedTime + STATS_UPDATE_INTERVAL) {
          statsLastUpdatedTime += STATS_UPDATE_INTERVAL;
        }
      }
    }
  }

  void joinThreads() {
    for (std::thread &thread : threads_) {
      thread.join();
    }
  }

  void finishSearch() {
    if (bestMove_ == Move::null()) {
      logWarn(JOB_RUNNER) << "The search didn't find anything; picking a random move";
      bestMove_ = pickRandomMove(position_.last);
    }
    server_.finishSearch(bestMove_);

    if (p_.isDebugMode()) {
      server_.sendString(
          "Total search time: " + std::to_string(timeElapsed(steady_clock::now()).count()) + " us");
    }
  }

  JobRunner &p_;
  const Position &position_;
  const size_t jobCount_;
  JobCommunicator &comm_;
  SoFBotApi::Server &server_;
  const steady_clock::time_point startTime_;
  const SearchLimits &limits_;

  // We store the jobs in `deque` instead of `vector`, as `Job` instances are not moveable
  std::deque<Job> jobs_;
  std::vector<std::thread> threads_;

  size_t bestDepth_ = 0;
  Move bestMove_ = Move::null();
  Stats stats_;
};

JobRunner::JobRunner(SoFBotApi::Server &server) : server_(server), evaluators_(DEFAULT_NUM_JOBS) {}

void JobRunner::clearHash() {
  std::unique_lock lock(applyConfigLock_);
  needClearHash_ = true;
  tryApplyConfigUnlocked();
}

void JobRunner::newGame() {
  std::unique_lock lock(applyConfigLock_);
  needNewGame_ = true;
  tryApplyConfigUnlocked();
}

void JobRunner::setHashSize(const size_t size) {
  std::unique_lock lock(applyConfigLock_);
  hashSize_ = size;
  tryApplyConfigUnlocked();
}

void JobRunner::setNumJobs(const size_t jobs) {
  std::unique_lock lock(applyConfigLock_);
  numJobs_ = jobs;
  tryApplyConfigUnlocked();
}

void JobRunner::join() {
  if (mainThread_.joinable()) {
    comm_.stop();
    mainThread_.join();
  }
}

void JobRunner::tryApplyConfigUnlocked() {
  if (!canApplyConfig_) {
    return;
  }
  if (evaluators_.size() != numJobs_) {
    evaluators_.resize(numJobs_);
  }
  if (needClearHash_ || tt_.sizeBytes() != hashSize_) {
    if (needClearHash_) {
      lastPosition_ = std::nullopt;
    }
    tt_.resize(hashSize_, needClearHash_, numJobs_);
    needClearHash_ = false;
    hashSize_ = tt_.sizeBytes();
  }
  if (needNewGame_) {
    needNewGame_ = false;
    if (lastPosition_) {
      lastPosition_ = std::nullopt;
      tt_.resetEpoch();
    }
  }
}

void JobRunner::start(const Position &position, const SearchLimits &limits) {
  join();
  comm_.reset(limits);
  setPosition(position);
  mainThread_ = std::thread([this, position, jobCount = this->numJobs_]() {
    MainThread mt(*this, position, jobCount);
    mt.run();
  });
}

void JobRunner::setPosition(Position position) {
  if (!lastPosition_) {
    lastPosition_ = std::move(position);
    return;
  }
  const size_t common = commonPrefix(*lastPosition_, position);
  if (common != COMMON_PREFIX_NONE) {
    const size_t diff = SoFUtil::absDiff(common, lastPosition_->moves.size()) +
                        SoFUtil::absDiff(common, position.moves.size());
    if (diff == 0) {
      // Do nothing
    } else if (diff <= 5) {
      tt_.growEpoch(static_cast<uint8_t>(diff));
    } else {
      tt_.resetEpoch();
    }
  } else {
    tt_.resetEpoch();
  }
  lastPosition_ = std::move(position);
}

void JobRunner::stop() { comm_.stop(); }

JobRunner::~JobRunner() { join(); }

}  // namespace SoFSearch::Private
