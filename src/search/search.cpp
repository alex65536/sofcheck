// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

#include "search/search.h"

#include <optional>
#include <vector>

#include "core/board.h"
#include "core/move.h"
#include "search/private/job_runner.h"
#include "search/private/limits.h"
#include "search/private/types.h"
#include "util/logging.h"
#include "util/misc.h"
#include "version/version.h"

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
      .addInt("Threads", 1, Private::JobRunner::DEFAULT_NUM_JOBS, 512)
      .addAction("Clear hash")
      .options();
}

ApiResult Engine::newGame() { return ApiResult::Ok; }

const char *Engine::name() const {
  static const std::string ENGINE_NAME =
      std::string("SoFCheck alpha (version ") + SoFVersion::GIT_VERSION + ")";
  return ENGINE_NAME.c_str();
}

const char *Engine::author() const { return "Alexander Kernozhitsky"; }

void Engine::enterDebugMode() { p_->runner->setDebugMode(true); }

void Engine::leaveDebugMode() { p_->runner->setDebugMode(false); }

ApiResult Engine::reportError(const char *message) {
  logError(ENGINE) << "Got server error: " << message;
  return ApiResult::Ok;
}

ApiResult Engine::doSearch(const Private::SearchLimits &limits) {
  p_->runner->start(p_->position, limits);
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
    if (value <= 0) {
      return ApiResult::InvalidArgument;
    }
    p_->runner->hashResize(static_cast<size_t>(value) << 20);
  } else if (key == "Threads") {
    if (value <= 0) {
      return ApiResult::InvalidArgument;
    }
    p_->runner->setNumJobs(static_cast<size_t>(value));
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
