#include "search/search.h"

#include <chrono>
#include <optional>
#include <thread>
#include <vector>

#include "core/board.h"
#include "core/move.h"
#include "search/private/job.h"
#include "util/logging.h"
#include "util/misc.h"

namespace SoFSearch {

using namespace std::chrono_literals;

using SoFBotApi::TimeControl;
using SoFCore::Board;
using SoFCore::Move;
using SoFUtil::logError;
using SoFUtil::panic;
using std::chrono::steady_clock;

// Type of log entry
constexpr const char *ENGINE = "Engine";

class EnginePrivate {
public:
  void setPosition(const Board &board, const SoFCore::Move *moves, size_t count) {
    board_ = board;
    moves_.assign(moves, moves + count);
  }

  void start() {
    clear();
    job_.emplace(control_, d_->server_);
    threads_.emplace_back([&]() {
      // Statistics thread
      auto lastStatsUpdated = steady_clock::now();
      while (!control_.isStopped()) {
        std::this_thread::sleep_for(30ms);
        auto now = steady_clock::now();
        if (now - lastStatsUpdated > 2s) {
          d_->server_->sendNodeCount(job_->stats().nodes());
          lastStatsUpdated = now;
        }
      }
    });
    threads_.emplace_back([board = board_, moves = moves_, &job = this->job_]() {
      // Job thread
      job->run(board, moves.data(), moves.size());
    });
    hasJob_ = true;
  }

  void stop() { control_.stop(); }

  void clear() {
    if (!hasJob_) {
      return;
    }
    control_.stop();
    for (std::thread &thread : threads_) {
      thread.join();
    }
    threads_.clear();
    control_.reset();
    job_.reset();
    hasJob_ = false;
  }

  explicit EnginePrivate(Engine *engine)
      : d_(engine), board_(Board::initialPosition()), job_(std::nullopt), hasJob_(false) {}

private:
  Engine *d_;
  Board board_;
  std::vector<Move> moves_;
  std::vector<std::thread> threads_;
  Private::JobControl control_;
  std::optional<Private::Job> job_;
  bool hasJob_;
};

ApiResult Engine::connect(Server *server) {
  server_ = server;
  return ApiResult::Ok;
}

void Engine::disconnect() {
  // We must stop the search and clear the threads on disconnect
  p_->clear();
  server_ = nullptr;
}

Engine::Engine()
    : options_(makeOptions(this)), server_(nullptr), p_(std::make_unique<EnginePrivate>(this)) {}

Engine::~Engine() {
  if (SOF_UNLIKELY(server_)) {
    panic("Server was not disconnected properly");
  }
}

SoFBotApi::OptionStorage Engine::makeOptions(Engine *engine) {
  return SoFBotApi::OptionBuilder(engine).options();
}

ApiResult Engine::newGame() { return ApiResult::Ok; }

ApiResult Engine::reportError(const char *message) {
  logError(ENGINE) << "Got server error: " << message;
  return ApiResult::Ok;
}

ApiResult Engine::searchInfinite() {
  p_->start();
  return ApiResult::Ok;
}

ApiResult Engine::searchTimeControl(const TimeControl &control) {
  // TODO : implement
  SOF_UNUSED(control);
  return ApiResult::NotSupported;
}

ApiResult Engine::setPosition(const SoFCore::Board &board, const SoFCore::Move *moves,
                              size_t count) {
  p_->setPosition(board, moves, count);
  return ApiResult::Ok;
}

ApiResult Engine::stopSearch() {
  p_->stop();
  return ApiResult::Ok;
}

ApiResult Engine::setBool(const std::string &, bool) { return ApiResult::Ok; }
ApiResult Engine::setEnum(const std::string &, size_t) { return ApiResult::Ok; }
ApiResult Engine::setInt(const std::string &, int64_t) { return ApiResult::Ok; }
ApiResult Engine::setString(const std::string &, const std::string &) { return ApiResult::Ok; }
ApiResult Engine::triggerAction(const std::string &) { return ApiResult::Ok; }

}  // namespace SoFSearch
