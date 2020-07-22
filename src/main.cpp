#include <iostream>
#include <string>
#include <utility>

#include "core/init.h"
#include "engine_base/client.h"
#include "engine_base/connection.h"
#include "engine_base/strutil.h"
#include "engine_clients/uci.h"
#include "util/misc.h"
#include "util/no_copy_move.h"

using SoFEngineBase::ApiResult;
using std::cerr;
using std::endl;

class TestEngine : public SoFEngineBase::Client, public SoFUtil::NoCopyMove {
public:
  const char *name() const override { return "Test Engine"; }
  const char *author() const override { return "ubilso ap stenku"; }

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
    cerr << "searchFixedNodes(" << time.count() << ")" << endl;
    return ApiResult::Ok;
  }

  ApiResult searchInfinite() override {
    cerr << "searchInfinite()" << endl;
    return ApiResult::Ok;
  }

  ApiResult searchTimeControl(const SoFEngineBase::TimeControl &control) override {
    cerr << "searchTimeControl(" << control.white.time.count() << ", " << control.white.inc.count()
         << ", " << control.black.time.count() << ", " << control.black.inc.count() << ")" << endl;
    if (control.movesToGo != SoFEngineBase::INFINITE_MOVES) {
      cerr << "movesToGo " << control.movesToGo;
    }
    return ApiResult::Ok;
  }

  ApiResult setPosition(const SoFCore::Board &board, const SoFCore::Move * /* moves */,
                        size_t count) override {
    cerr << "setPosition(" << board.asFen() << ", count = " << count << ")" << endl;
    return ApiResult::Ok;
  }

  ApiResult stopSearch() override {
    cerr << "stopSearch()" << endl;
    server_->sendNodeCount(42'000'000);
    server_->sendString(":)");
    server_->finishSearch(SoFCore::Move{SoFCore::MoveKind::PawnDoubleMove, 52, 36, 0});
    return ApiResult::Ok;
  }

private:
  ApiResult connect(SoFEngineBase::Server *server) override {
    server_ = server;
    return ApiResult::Ok;
  }

  void disconnect() override { server_ = nullptr; }

  SoFEngineBase::Server *server_;
};

int main(int argc, char **argv) {
  using SoFEngineBase::Connection;
  using SoFEngineBase::PollResult;
  using SoFEngineClients::UciServerConnector;
  using SoFUtil::panic;

  unused(argc);
  unused(argv);
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
