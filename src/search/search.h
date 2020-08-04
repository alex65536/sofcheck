#ifndef SOF_SEARCH_SEARCH_INCLUDED
#define SOF_SEARCH_SEARCH_INCLUDED

#include <chrono>
#include <memory>

#include "bot_api/client.h"
#include "bot_api/options.h"

namespace SoFSearch {

using SoFBotApi::ApiResult;
using SoFBotApi::Options;
using SoFBotApi::Server;

class EnginePrivate;

// The chess engine class which uses `SoFBotApi::Client` as an interface
class Engine final : public SoFBotApi::Client, private SoFBotApi::OptionObserver {
public:
  virtual const char *name() const override { return "SoFCheck pre-alpha"; }
  const char *author() const override { return "SoFCheck developers"; }

  Options &options() override { return options_; }
  const Options &options() const override { return options_; }

  // TODO : implement
  // void enterDebugMode() override;
  // void leaveDebugMode() override;

  ApiResult newGame() override;

  ApiResult setPosition(const SoFCore::Board &board, const SoFCore::Move *moves,
                        size_t count) override;

  ApiResult searchInfinite() override;
  ApiResult searchFixedDepth(size_t depth) override;
  ApiResult searchFixedNodes(uint64_t nodes) override;
  ApiResult searchFixedTime(std::chrono::milliseconds time) override;
  ApiResult searchTimeControl(const SoFBotApi::TimeControl &control) override;

  ApiResult stopSearch() override;
  ApiResult reportError(const char *message) override;

  Engine();
  ~Engine();

private:
  ApiResult connect(Server *server) override;
  void disconnect() override;

  ApiResult setBool(const std::string &key, bool value) override;
  ApiResult setEnum(const std::string &key, size_t index) override;
  ApiResult setInt(const std::string &key, int64_t value) override;
  ApiResult setString(const std::string &key, const std::string &value) override;
  ApiResult triggerAction(const std::string &key) override;

private:
  static SoFBotApi::OptionStorage makeOptions(Engine *engine);

  friend class EnginePrivate;

  SoFBotApi::OptionStorage options_;
  Server *server_;
  std::unique_ptr<EnginePrivate> p_;
};

}  // namespace SoFSearch

#endif  // SOF_SEARCH_SEARCH_INCLUDED
