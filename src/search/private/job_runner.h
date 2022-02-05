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

#ifndef SOF_SEARCH_PRIVATE_JOB_RUNNER_INCLUDED
#define SOF_SEARCH_PRIVATE_JOB_RUNNER_INCLUDED

#include <atomic>
#include <cstddef>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "eval/score.h"
#include "search/private/job.h"
#include "search/private/transposition_table.h"
#include "search/private/types.h"

namespace SoFBotApi {
class Server;
}  // namespace SoFBotApi

namespace SoFSearch::Private {

struct SearchLimits;

// The class that runs multiple search jobs simultaneously and controls them.
class JobRunner {
public:
  // Default number of jobs for `JobRunner`
  static constexpr size_t DEFAULT_NUM_JOBS = 1;

  explicit JobRunner(SoFBotApi::Server &server);
  ~JobRunner();

  // Stops the search. This operation is asynchronous, so the jobs may work for some time after you
  // call this function.
  void stop();

  // Stops the search and waits until it is really stopped. If the search is not started, this
  // function does nothing.
  void join();

  // Starts the search. If the search is already started, the previous search is stopped in a
  // blocked manner (i.e. by calling `join()`)
  void start(const Position &position, const SearchLimits &limits);

  // Indicates that the hash table size (in bytes) must be changed to `size`. The resize operation
  // may be deferred until the search is stopped.
  void setHashSize(size_t size);

  // Indicates that the hash table must be cleared. The clear operation may be deferred until the
  // search is stopped.
  void clearHash();

  // Indicates that the following searches will use positions from a new game
  void newGame();

  // Returns the number of jobs to run
  inline size_t numJobs() const { return numJobs_; }

  // Sets the number of jobs to run. If the search is already running, the change will be applied
  // only for the next search
  void setNumJobs(size_t jobs);

  // Enables or disables debug mode. In debug mode the jobs may send extra information to server.
  inline void setDebugMode(const bool enable) {
    debugMode_.store(enable, std::memory_order_release);
  }

  // Returns `true` if debug mode is enabled
  inline bool isDebugMode() const { return debugMode_.load(std::memory_order_acquire); }

private:
  // Main function of the thread which controls all the running jobs.
  void runMainThread(const Position &position, size_t jobCount);

  // Does nothing if the search is running (i.e. `canApplyConfig_` is `false`). Otherwise, applies
  // new configuration if it has changed since last successful call to this function.  Must be
  // called under `applyConfigLock_`
  void tryApplyConfigUnlocked();

  // Helper method. Acknowledges that search in the position `position` is being started
  void setPosition(Position position);

  JobCommunicator comm_;
  TranspositionTable tt_;
  SoFBotApi::Server &server_;
  std::vector<SoFEval::ScoreEvaluator> evaluators_;

  std::thread mainThread_;
  std::mutex applyConfigLock_;
  std::atomic<bool> debugMode_ = false;
  bool canApplyConfig_ = true;

  std::optional<Position> lastPosition_;

  size_t hashSize_ = TranspositionTable::DEFAULT_SIZE;
  size_t numJobs_ = DEFAULT_NUM_JOBS;
  bool needClearHash_ = false;
  bool needNewGame_ = false;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_JOB_RUNNER_INCLUDED
