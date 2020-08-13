#include "search/private/job_runner.h"

#include <chrono>
#include <deque>
#include <string>
#include <utility>
#include <vector>

#include "util/defer.h"

namespace SoFSearch::Private {

using namespace std::chrono_literals;

using SoFCore::Board;
using SoFCore::Move;
using std::chrono::steady_clock;

struct Stats {
  uint64_t nodes = 0;
  uint64_t ttHits = 0;

  void add(const JobResults &results) {
    nodes += results.nodes();
    ttHits += results.ttHits();
  }
};

void JobRunner::hashClear() {
  std::unique_lock lock(hashChangeLock_);
  if (!canChangeHash_) {
    clearHash_ = true;
    return;
  }
  tt_.clear();
}

void JobRunner::hashResize(const size_t size) {
  std::unique_lock lock(hashChangeLock_);
  if (!canChangeHash_) {
    hashSize_ = size;
    return;
  }
  tt_.resize(size, false);
  hashSize_ = tt_.sizeBytes();
}

void JobRunner::join() {
  if (mainThread_.joinable()) {
    comm_.stop();
    mainThread_.join();
  }
}

void JobRunner::runMainThread(const Board &board, const std::vector<Move> &moves,
                              const SearchLimits &limits, const size_t numJobs) {
  {
    std::unique_lock lock(hashChangeLock_);
    canChangeHash_ = false;
  }
  SOF_DEFER({
    // Perform delayed requests to modify transposition table
    std::unique_lock lock(hashChangeLock_);
    tt_.resize(hashSize_, clearHash_);
    hashSize_ = tt_.sizeBytes();
    clearHash_ = false;
    canChangeHash_ = true;
  });

  // Create jobs and associated threads. We store the jobs in `deque` instead of `vector`, as `Job`
  // instances are not moveable
  std::deque<Job> jobs;
  for (size_t i = 0; i < numJobs; ++i) {
    jobs.emplace_back(comm_, tt_, server_, i);
  }
  std::vector<std::thread> threads;
  for (size_t i = 0; i < numJobs; ++i) {
    threads.emplace_back(
        [&job = jobs[i], &board, &moves, &limits]() { job.run(board, moves, limits); });
  }

  static constexpr auto STATS_UPDATE_INTERVAL = 3s;
  static constexpr auto THREAD_TICK_INTERVAL = 30ms;

  // Run loop in which we check the jobs' status
  const auto startTime = steady_clock::now();
  auto statsLastUpdatedTime = startTime;
  while (!comm_.isStopped()) {
    std::this_thread::sleep_for(THREAD_TICK_INTERVAL);
    const auto now = steady_clock::now();

    // Collect stats
    Stats stats;
    for (const Job &job : jobs) {
      stats.add(job.results());
    }

    // Check if it's time to stop
    if (stats.nodes > limits.nodes ||
        (limits.time != TIME_UNLIMITED && now - startTime > limits.time)) {
      comm_.stop();
    }

    // Print stats
    if (now >= statsLastUpdatedTime + STATS_UPDATE_INTERVAL) {
      server_.sendNodeCount(stats.nodes);
      if (isDebugMode()) {
        server_.sendString("Hash table hits: " + std::to_string(stats.ttHits));
      }
      while (now >= statsLastUpdatedTime + STATS_UPDATE_INTERVAL) {
        statsLastUpdatedTime += STATS_UPDATE_INTERVAL;
      }
    }
  }

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
  server_.finishSearch(bestMove);
}

void JobRunner::start(const Board &board, const std::vector<Move> &moves,
                      const SearchLimits &limits, const size_t numJobs) {
  join();
  comm_.reset();
  mainThread_ = std::thread(
      [this, board, moves, limits, numJobs]() { runMainThread(board, moves, limits, numJobs); });
}

void JobRunner::stop() { comm_.stop(); }

JobRunner::~JobRunner() { join(); }

}  // namespace SoFSearch::Private
