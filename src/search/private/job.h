#ifndef SOF_SEARCH_PRIVATE_JOB_INCLUDED
#define SOF_SEARCH_PRIVATE_JOB_INCLUDED

#include <atomic>
#include <cstdint>

#include "bot_api/server.h"
#include "core/board.h"
#include "core/move.h"

namespace SoFSearch::Private {

class JobControl {
public:
  inline void stop() { stopped_.store(true, std::memory_order_relaxed); }
  inline void reset() { stopped_.store(false, std::memory_order_relaxed); }
  inline bool isStopped() const { return stopped_.load(std::memory_order_relaxed); }

private:
  std::atomic<bool> stopped_ = false;
};

// Statistics collector. This class assumes assume that there can be only one writer thread and
// infinitely many reader threads.
class JobStats {
public:
  inline uint64_t nodes() const { return nodes_.load(std::memory_order_relaxed); }

  inline void incNodes() { incRelaxed(nodes_); }

private:
  inline static void incRelaxed(std::atomic<uint64_t> &value) {
    const uint64_t newValue = value.load(std::memory_order_relaxed) + 1;
    value.store(newValue, std::memory_order_relaxed);
  }

  std::atomic<uint64_t> nodes_ = 0;
};

class Job {
public:
  Job(JobControl &control, SoFBotApi::Server *server) : control_(control), server_(server) {}

  inline constexpr const JobStats &stats() const { return stats_; }

  void run(SoFCore::Board board, const SoFCore::Move *moves, size_t count);

private:
  JobControl &control_;
  JobStats stats_;
  SoFBotApi::Server *server_;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_JOB_INCLUDED
