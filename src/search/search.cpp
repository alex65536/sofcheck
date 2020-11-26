#include "search/search.h"

#include <optional>

#include "core/board.h"
#include "core/move.h"
#include "search/private/job_runner.h"
#include "search/private/limits.h"
#include "search/private/types.h"
#include "util/logging.h"
#include "util/misc.h"

namespace SoFSearch {

using namespace std::chrono_literals;
using namespace SoFUtil::Logging;

using Private::Position;
using Private::SearchLimits;
using SoFBotApi::ApiResult;
using SoFBotApi::TimeControl;
using SoFCore::Board;
using SoFCore::Move;
using SoFUtil::panic;

// Type of log entry
constexpr const char *ENGINE = "Engine";

struct Engine::Impl {
  std::optional<Private::JobRunner> runner;
  Position position = Position::from(Board::initialPosition(), {});
  std::vector<Move> moves;
};

ApiResult Engine::connect(SoFBotApi::Server *server) {
  server_ = server;
  p_->runner.emplace(*server_);
  return ApiResult::Ok;
}

void Engine::disconnect() {
  p_->runner->join();
  p_->runner.reset();
  server_ = nullptr;
}

Engine::Engine() : options_(makeOptions(this)), p_(std::make_unique<Impl>()) {}

Engine::~Engine() {
  if (SOF_UNLIKELY(server_)) {
    panic("Server was not disconnected properly");
  }
}

SoFBotApi::OptionStorage Engine::makeOptions(Engine *engine) {
  return SoFBotApi::OptionBuilder(engine)
      .addInt("Hash", 1, Private::TranspositionTable::DEFAULT_SIZE >> 20, 131'072)
      .addInt("Threads", 1, 1, 512)
      .addAction("Clear hash")
      .options();
}

ApiResult Engine::newGame() {p_->runner->hashClear(); return ApiResult::Ok; }

void Engine::enterDebugMode() { p_->runner->setDebugMode(true); }

void Engine::leaveDebugMode() { p_->runner->setDebugMode(false); }

ApiResult Engine::reportError(const char *message) {
  logError(ENGINE) << "Got server error: " << message;
  return ApiResult::Ok;
}

ApiResult Engine::doSearch(const Private::SearchLimits &limits) {
  p_->runner->start(p_->position, limits, options_.getInt("Threads")->value);
  return ApiResult::Ok;
}

ApiResult Engine::searchInfinite() { return doSearch(SearchLimits::withInfiniteTime()); }

ApiResult Engine::searchFixedDepth(size_t depth) {
  return doSearch(SearchLimits::withFixedDepth(depth));
}

ApiResult Engine::searchFixedNodes(uint64_t nodes) {
  return doSearch(SearchLimits::withFixedNodes(nodes));
}

ApiResult Engine::searchFixedTime(std::chrono::milliseconds time) {
  return doSearch(SearchLimits::withFixedTime(time));
}

ApiResult Engine::searchTimeControl(const TimeControl &control) {
  return doSearch(SearchLimits::withTimeControl(p_->position.last, control));
}

ApiResult Engine::setPosition(const SoFCore::Board &board, const SoFCore::Move *moves,
                              size_t count) {
  p_->position = Position::from(board, std::vector<Move>(moves, moves + count));
  return ApiResult::Ok;
}

ApiResult Engine::stopSearch() {
  p_->runner->stop();
  return ApiResult::Ok;
}

ApiResult Engine::setBool(const std::string &, bool) { return ApiResult::Ok; }
ApiResult Engine::setEnum(const std::string &, size_t) { return ApiResult::Ok; }

ApiResult Engine::setInt(const std::string &key, const int64_t value) {
  if (key == "Hash") {
    p_->runner->hashResize(value << 20);
  }
  return ApiResult::Ok;
}

ApiResult Engine::setString(const std::string &, const std::string &) { return ApiResult::Ok; }

ApiResult Engine::triggerAction(const std::string &key) {
  if (key == "Clear hash") {
    p_->runner->hashClear();
  }
  return ApiResult::Ok;
}

}  // namespace SoFSearch
