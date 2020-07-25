#include "engine_clients/uci.h"

#include <iostream>
#include <string>
#include <utility>

#include "core/init.h"
#include "core/strutil.h"
#include "engine_base/client.h"
#include "engine_base/connection.h"
#include "engine_base/options.h"
#include "engine_base/strutil.h"
#include "util/misc.h"
#include "util/no_copy_move.h"

using SoFEngineBase::ApiResult;
using SoFEngineBase::Connection;
using SoFEngineBase::PollResult;
using SoFEngineBase::PositionCost;
using SoFEngineBase::PositionCostBound;
using SoFEngineClients::UciServerConnector;
using SoFUtil::panic;
using std::cerr;
using std::endl;

class TestEngine : public SoFEngineBase::Client,
                   public SoFUtil::NoCopyMove,
                   private SoFEngineBase::OptionObserver {
public:
  const char *name() const override { return "Test Engine"; }
  const char *author() const override { return "Test Author"; }

  ApiResult reportError(const char *message) override {
    cerr << "reportError(" << message << ")" << endl;
    return ApiResult::Ok;
  }

  void enterDebugMode() override { cerr << "enterDebugMode()" << endl; }

  void leaveDebugMode() override { cerr << "leaveDebugMode()" << endl; }

  ApiResult newGame() override {
    cerr << "newGame()" << endl;
    return ApiResult::Ok;
  }

  ApiResult searchFixedDepth(size_t depth) override {
    cerr << "searchFixedDepth(" << depth << ")" << endl;
    return ApiResult::Ok;
  }

  ApiResult searchFixedNodes(uint64_t nodes) override {
    cerr << "searchFixedNodes(" << nodes << ")"
         << " // not supported" << endl;
    return ApiResult::NotSupported;
  }

  ApiResult searchFixedTime(std::chrono::milliseconds time) override {
    cerr << "searchFixedTime(" << time.count() << ")" << endl;
    std::vector<SoFCore::Move> moves{SoFCore::Move{SoFCore::MoveKind::PawnDoubleMove, 52, 36, 0}};
    server_->sendResult(
        {10, moves.data(), moves.size(), PositionCost::centipawns(100), PositionCostBound::Exact});
    server_->sendResult({15, moves.data(), moves.size(), PositionCost::checkMate(-1),
                         PositionCostBound::Lowerbound});
    server_->sendResult({20, moves.data(), moves.size(), PositionCost::checkMate(5),
                         PositionCostBound::Upperbound});
    return ApiResult::Ok;
  }

  ApiResult searchInfinite() override {
    cerr << "searchInfinite()" << endl;
    return ApiResult::Ok;
  }

  ApiResult searchTimeControl(const SoFEngineBase::TimeControl &control) override {
    cerr << "searchTimeControl(" << control.white.time.count() << ", " << control.white.inc.count()
         << ", " << control.black.time.count() << ", " << control.black.inc.count();
    if (control.movesToGo != SoFEngineBase::INFINITE_MOVES) {
      cerr << ", movesToGo = " << control.movesToGo;
    }
    cerr << ")" << endl;
    return ApiResult::Ok;
  }

  ApiResult setPosition(const SoFCore::Board &board, const SoFCore::Move *moves,
                        size_t count) override {
    cerr << "setPosition(" << board.asFen();
    for (size_t i = 0; i < count; ++i) {
      cerr << ", " << SoFCore::moveToStr(moves[i]);
    }
    cerr << ")" << endl;
    return ApiResult::Ok;
  }

  ApiResult stopSearch() override {
    cerr << "stopSearch()" << endl;
    // Do not send nodeCount, as it would also print time, which may change from test to test
    // server_->sendNodeCount(42'000'000);
    server_->sendString(":)");
    server_->sendHashFull(500);
    server_->finishSearch(SoFCore::Move{SoFCore::MoveKind::PawnDoubleMove, 52, 36, 0});
    return ApiResult::Ok;
  }

  SoFEngineBase::Options &options() override { return options_; }

  const SoFEngineBase::Options &options() const override { return options_; }

  TestEngine() : options_(buildOptions(this)) {}

private:
  ApiResult connect(SoFEngineBase::Server *server) override {
    server_ = server;
    return ApiResult::Ok;
  }

  ApiResult setBool(const std::string &key, bool value) override {
    cerr << "setBool(" << key << ", " << value << ")" << endl;
    return ApiResult::Ok;
  }

  ApiResult setEnum(const std::string &key, size_t index) override {
    cerr << "setEnum(" << key << ", " << index << ")" << endl;
    return ApiResult::Ok;
  }

  SoFEngineBase::ApiResult setInt(const std::string &key, int64_t value) override {
    cerr << "setInt(" << key << ", " << value << ")" << endl;
    if (key == "int" && value == 42) {
      return ApiResult::RuntimeError;
    }
    return ApiResult::Ok;
  }

  SoFEngineBase::ApiResult setString(const std::string &key, const std::string &value) override {
    cerr << "setString(" << key << ", " << value << ")" << endl;
    return ApiResult::Ok;
  }

  SoFEngineBase::ApiResult triggerAction(const std::string &key) override {
    cerr << "triggerAction(" << key << ")" << endl;
    return ApiResult::Ok;
  }

  void disconnect() override { server_ = nullptr; }

  static SoFEngineBase::Options buildOptions(SoFEngineBase::OptionObserver *observer) {
    return SoFEngineBase::OptionBuilder(observer)
        .addBool("true bool", true)
        .addBool("false bool", false)
        .addBool("name value", true)
        .addEnum("enum", {"name", "value", "val", "my name", "my val"}, 1)
        .addEnum("enum value", {"v1", "v3", "v2"}, 0)
        .addInt("int", 0, 10, 100)
        .addInt("int 2", -100, 10, 100)
        .addString("empty string", "")
        .addString("good string", "42")
        .addAction("name val")
        .addAction("_name _value")
        .addAction("__name __value")
        .addAction("good")
        .options();
  }

  SoFEngineBase::Options options_;
  SoFEngineBase::Server *server_ = nullptr;
};

int main() {
  SoFCore::init();

  auto connResult = Connection::clientSide<TestEngine, UciServerConnector>();
  if (connResult.isErr()) {
    panic(std::string("Connection failed: ") +
          SoFEngineBase::apiResultToStr(connResult.unwrapErr()));
  }
  auto connection = connResult.unwrap();
  PollResult result = connection.runPollLoop();
  if (result != PollResult::Ok) {
    panic(std::string("Poll failed: ") + SoFEngineBase::pollResultToStr(result));
  }

  return 0;
}
