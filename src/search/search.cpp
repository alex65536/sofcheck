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

#include "search/search.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "bot_api/api_base.h"
#include "bot_api/options.h"
#include "config.h"
#include "core/board.h"
#include "core/move.h"
#include "search/private/job_runner.h"
#include "search/private/limits.h"
#include "search/private/types.h"
#include "util/logging.h"
#include "util/misc.h"
#include "util/strutil.h"
#include "version/version.h"

namespace SoFSearch {

using namespace SoFUtil::Logging;

using Private::Position;
using Private::SearchLimits;
using SoFBotApi::ApiResult;
using SoFBotApi::Options;
using SoFBotApi::TimeControl;
using SoFCore::Board;
using SoFCore::Move;

// Type of log entry
constexpr const char *ENGINE = "Engine";

class Engine final : public SoFBotApi::Client, private SoFBotApi::OptionObserver {
public:
  const char *name() const override {
    static const std::string ENGINE_NAME = []() {
      const std::string rawVersion = SoFVersion::GIT_VERSION;
      std::string version;
      if (rawVersion == "unknown" || SoFUtil::startsWith(rawVersion, "v")) {
        version = rawVersion;
      } else {
        version = "v0.0-g" + rawVersion;
      }
      return std::string("SoFCheck [") + version + " " + CPU_ARCH_FULL + "]";
    }();
    return ENGINE_NAME.c_str();
  }

  const char *author() const override { return "Alexander Kernozhitsky"; }

  Options &options() override { return options_; }
  const Options &options() const override { return options_; }

  void enterDebugMode() override { runner_->setDebugMode(true); }
  void leaveDebugMode() override { runner_->setDebugMode(false); }

  ApiResult newGame() override {
    runner_->newGame();
    return ApiResult::Ok;
  }

  ApiResult setPosition(const Board &board, const Move *moves, const size_t count) override {
    position_ = Position::from(board, std::vector<Move>(moves, moves + count));
    return ApiResult::Ok;
  }

  ApiResult searchInfinite() override { return doSearch(SearchLimits::withInfiniteTime()); }

  ApiResult searchFixedDepth(const size_t depth) override {
    return doSearch(SearchLimits::withFixedDepth(depth));
  }

  ApiResult searchFixedNodes(const uint64_t nodes) override {
    return doSearch(SearchLimits::withFixedNodes(nodes));
  }

  ApiResult searchFixedTime(const std::chrono::milliseconds time) override {
    return doSearch(SearchLimits::withFixedTime(time));
  }

  ApiResult searchTimeControl(const TimeControl &control) override {
    return doSearch(SearchLimits::withTimeControl(position_.last, control));
  }

  ApiResult stopSearch() override {
    runner_->stop();
    return ApiResult::Ok;
  }

  ApiResult reportError(const char *message) override {
    logError(ENGINE) << "Got server error: " << message;
    return ApiResult::Ok;
  }

  Engine() : options_(makeOptions(this)) {}

  ~Engine() override { SOF_ASSERT_MSG("Server was not disconnected properly", !server_); }

private:
  ApiResult connect(SoFBotApi::Server *server) override {
    server_ = server;
    runner_.emplace(*server_);
    return ApiResult::Ok;
  }

  void disconnect() override {
    runner_->join();
    runner_.reset();
    server_ = nullptr;
  }

  ApiResult setBool(const std::string &, bool) override { return ApiResult::Ok; }
  ApiResult setEnum(const std::string &, size_t) override { return ApiResult::Ok; }
  ApiResult setString(const std::string &, const std::string &) override { return ApiResult::Ok; }

  ApiResult setInt(const std::string &key, const int64_t value) override {
    if (key == "Hash") {
      if (value <= 0) {
        return ApiResult::InvalidArgument;
      }
      runner_->setHashSize(static_cast<size_t>(value) << 20);
    } else if (key == "Threads") {
      if (value <= 0) {
        return ApiResult::InvalidArgument;
      }
      runner_->setNumJobs(static_cast<size_t>(value));
    }
    return ApiResult::Ok;
  }

  ApiResult triggerAction(const std::string &key) override {
    if (key == "Clear hash") {
      runner_->clearHash();
    }
    return ApiResult::Ok;
  }

  static SoFBotApi::OptionStorage makeOptions(Engine *engine) {
    return SoFBotApi::OptionBuilder(engine)
        .addInt("Hash", 1, Private::TranspositionTable::DEFAULT_SIZE >> 20, 131'072)
        .addInt("Threads", 1, Private::JobRunner::DEFAULT_NUM_JOBS, 512)
        .addAction("Clear hash")
        .options();
  }

  ApiResult doSearch(const Private::SearchLimits &limits) {
    runner_->start(position_, limits);
    return ApiResult::Ok;
  }

  SoFBotApi::OptionStorage options_;
  SoFBotApi::Server *server_ = nullptr;
  std::optional<Private::JobRunner> runner_;
  Position position_ = Position::from(Board::initialPosition(), {});
};

std::unique_ptr<SoFBotApi::Client> makeEngine() { return std::make_unique<Engine>(); }

}  // namespace SoFSearch
