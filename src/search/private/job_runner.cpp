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

  inline void add(const JobResults &results) {
    for (size_t i = 0; i < JOB_STAT_SZ; ++i) {
      stats_[i] += results.get(static_cast<JobStat>(i));
    }
  }

private:
  uint64_t stats_[JOB_STAT_SZ] = {};
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

void JobRunner::runMainThread(const Position &position, const size_t jobCount) {
  {
    std::unique_lock lock(applyConfigLock_);
    canApplyConfig_ = false;
  }
  SOF_DEFER({
    // Perform delayed reconfiguration
    std::unique_lock lock(applyConfigLock_);
    canApplyConfig_ = true;
    tryApplyConfigUnlocked();
  });

  // Create jobs and associated threads. We store the jobs in `deque` instead of `vector`, as `Job`
  // instances are not moveable
  SOF_ASSERT(evaluators_.size() == jobCount);
  std::deque<Job> jobs;
  for (size_t i = 0; i < jobCount; ++i) {
    jobs.emplace_back(comm_, tt_, server_, evaluators_[i], i);
  }
  std::vector<std::thread> threads;
  for (size_t i = 0; i < jobCount; ++i) {
    threads.emplace_back([&job = jobs[i], &position]() { job.run(position); });
  }

  static constexpr microseconds STATS_UPDATE_INTERVAL = 3s;
  static constexpr microseconds THREAD_TICK_INTERVAL = 30ms;

  const auto startTime = comm_.startTime();
  const SearchLimits &limits = comm_.limits();

  const auto calcSleepTime = [&]() {
    if (limits.time == TIME_UNLIMITED) {
      return THREAD_TICK_INTERVAL;
    }
    const auto now = steady_clock::now();
    const auto timePassed = now - startTime;
    const auto timeLeft = duration_cast<microseconds>(limits.time - timePassed);
    const auto delay = std::min(timeLeft + 100us, THREAD_TICK_INTERVAL);
    return std::max(delay, 100us);
  };

  // Run loop in which we check the jobs' status
  auto statsLastUpdatedTime = startTime;
  do {
    const auto now = steady_clock::now();

    // Collect stats
    Stats stats;
    for (const Job &job : jobs) {
      stats.add(job.results());
    }

    // Check if it's time to stop
    if (stats.nodes() > limits.nodes ||
        (limits.time != TIME_UNLIMITED && now - startTime > limits.time)) {
      comm_.stop();
    }

    // Print stats
    if (now >= statsLastUpdatedTime + STATS_UPDATE_INTERVAL) {
      server_.sendNodeCount(stats.nodes());
      server_.sendHashHits(stats.get(JobStat::TtHits));
      if (isDebugMode()) {
        std::ostringstream nodeStream;
        nodeStream << "Node counts: P = " << stats.pvNodes() << " N = " << stats.nonPvNodes()
                   << " O = " << stats.otherNodes()
                   << " PI = " << stats.get(JobStat::PvInternalNodes)
                   << " NI = " << stats.get(JobStat::NonPvInternalNodes);
        std::ostringstream edgeStream;
        edgeStream.precision(3);
        edgeStream.flags(edgeStream.flags() | std::ostream::fixed);
        edgeStream << "Edge counts: PP = " << stats.get(JobStat::PPEdges)
                   << " PN = " << stats.get(JobStat::PNEdges)
                   << " NN = " << stats.get(JobStat::NNEdges)
                   << " PBranch = " << stats.pvBranchFactor()
                   << " NBranch = " << stats.nonPvBranchFactor()
                   << " Branch = " << stats.branchFactor();

        server_.sendString("Hash exact hits: " + std::to_string(stats.get(JobStat::TtExactHits)));
        server_.sendString(nodeStream.str());
        server_.sendString(edgeStream.str());
      }
      while (now >= statsLastUpdatedTime + STATS_UPDATE_INTERVAL) {
        statsLastUpdatedTime += STATS_UPDATE_INTERVAL;
      }
    }
  } while (!comm_.wait(calcSleepTime()));

  // Join job threads
  for (std::thread &thread : threads) {
    thread.join();
  }

  // Display best move
  size_t bestDepth = 0;
  Move bestMove = Move::null();
  for (const Job &job : jobs) {
    const size_t depth = job.results().depth();
    if (depth > bestDepth) {
      bestDepth = depth;
      bestMove = job.results().bestMove();
    }
  }
  if (bestMove == Move::null()) {
    if (bestDepth != 0) {
      logError(JOB_RUNNER) << "At least one depth is calculated, but the best move is not found";
    }
    bestMove = pickRandomMove(position.last);
  }
  server_.finishSearch(bestMove);

  if (isDebugMode()) {
    const auto endTime = std::chrono::steady_clock::now();
    const auto timeElapsed =
        std::chrono::duration_cast<std::chrono::microseconds>(endTime - comm_.startTime());
    server_.sendString("Total search time: " + std::to_string(timeElapsed.count()) + " us");
  }
}

void JobRunner::start(const Position &position, const SearchLimits &limits) {
  join();
  comm_.reset(limits);
  setPosition(position);
  mainThread_ = std::thread(
      [this, position, jobCount = this->numJobs_]() { runMainThread(position, jobCount); });
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
