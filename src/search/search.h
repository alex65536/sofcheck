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

#ifndef SOF_SEARCH_SEARCH_INCLUDED
#define SOF_SEARCH_SEARCH_INCLUDED

#include <chrono>
#include <memory>

#include "bot_api/client.h"
#include "bot_api/options.h"

namespace SoFSearch {

namespace Private {
struct SearchLimits;
}  // namespace Private

// The chess engine class which uses `SoFBotApi::Client` as an interface
class Engine final : public SoFBotApi::Client, private SoFBotApi::OptionObserver {
public:
  const char *name() const override;
  const char *author() const override;

  SoFBotApi::Options &options() override { return options_; }
  const SoFBotApi::Options &options() const override { return options_; }

  void enterDebugMode() override;
  void leaveDebugMode() override;

  SoFBotApi::ApiResult newGame() override;

  SoFBotApi::ApiResult setPosition(const SoFCore::Board &board, const SoFCore::Move *moves,
                                   size_t count) override;

  SoFBotApi::ApiResult searchInfinite() override;
  SoFBotApi::ApiResult searchFixedDepth(size_t depth) override;
  SoFBotApi::ApiResult searchFixedNodes(uint64_t nodes) override;
  SoFBotApi::ApiResult searchFixedTime(std::chrono::milliseconds time) override;
  SoFBotApi::ApiResult searchTimeControl(const SoFBotApi::TimeControl &control) override;

  SoFBotApi::ApiResult stopSearch() override;
  SoFBotApi::ApiResult reportError(const char *message) override;

  Engine();
  ~Engine() override;

private:
  SoFBotApi::ApiResult connect(SoFBotApi::Server *server) override;
  void disconnect() override;

  SoFBotApi::ApiResult setBool(const std::string &key, bool value) override;
  SoFBotApi::ApiResult setEnum(const std::string &key, size_t index) override;
  SoFBotApi::ApiResult setInt(const std::string &key, int64_t value) override;
  SoFBotApi::ApiResult setString(const std::string &key, const std::string &value) override;
  SoFBotApi::ApiResult triggerAction(const std::string &key) override;

  static SoFBotApi::OptionStorage makeOptions(Engine *engine);

  struct Impl;

  SoFBotApi::ApiResult doSearch(const Private::SearchLimits &limits);

  SoFBotApi::OptionStorage options_;
  SoFBotApi::Server *server_ = nullptr;
  std::unique_ptr<Impl> p_;
};

}  // namespace SoFSearch

#endif  // SOF_SEARCH_SEARCH_INCLUDED
