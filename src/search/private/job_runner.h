#ifndef SOF_SEARCH_PRIVATE_JOB_RUNNER_INCLUDED
#define SOF_SEARCH_PRIVATE_JOB_RUNNER_INCLUDED

#include <mutex>
#include <thread>

#include "bot_api/server.h"
#include "core/board.h"
#include "core/move.h"
#include "search/private/job.h"
#include "search/private/limits.h"
#include "search/private/transposition_table.h"

namespace SoFSearch::Private {

class JobRunner {
public:
  inline JobRunner(SoFBotApi::Server *server) : server_(server) {}

  ~JobRunner();

  void stop();
  void join();

  void start(const SoFCore::Board &board, const std::vector<SoFCore::Move> &moves,
             const SearchLimits &limits, const size_t numJobs);

  void hashResize(const size_t size);
  void hashClear();

private:
  void runMainThread(const SoFCore::Board &board, const std::vector<SoFCore::Move> &moves,
                     const SearchLimits &limits, const size_t numJobs);

  JobCommunicator comm_;
  TranspositionTable tt_;
  SoFBotApi::Server *server_;

  std::thread mainThread_;
  std::mutex hashChangeLock_;
  size_t hashSize_ = TranspositionTable::DEFAULT_SIZE;
  bool clearHash_ = false;
  bool canChangeHash_ = true;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_JOB_RUNNER_INCLUDED
