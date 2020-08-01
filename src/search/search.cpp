#include "search/search.h"

#include <iostream>

#include "util/misc.h"

namespace SoFSearch {

using SoFBotApi::TimeControl;
using SoFUtil::panic;
using std::cerr;
using std::endl;

ApiResult Engine::connect(Server *server) {
  server_ = server;
  return ApiResult::Ok;
}

void Engine::disconnect() { server_ = nullptr; }

Engine::Engine() : options_(makeOptions(this)), server_(nullptr) {}

Engine::~Engine() {
  if (SOF_UNLIKELY(server_)) {
    panic("Server was not disconnected properly");
  }
}

Options Engine::makeOptions(SoFBotApi::OptionObserver *observer) {
  return SoFBotApi::OptionBuilder(observer).options();
}

ApiResult Engine::newGame() { return ApiResult::Ok; }

ApiResult Engine::reportError(const char *message) {
  cerr << "Got server error: " << message << endl;
  return ApiResult::Ok;
}

ApiResult Engine::searchInfinite() {
  // TODO : implement
  return ApiResult::NotSupported;
}

ApiResult Engine::searchTimeControl(const TimeControl &control) {
  // TODO : implement
  SOF_UNUSED(control);
  return ApiResult::NotSupported;
}

ApiResult Engine::setPosition(const SoFCore::Board &board, const SoFCore::Move *moves,
                              size_t count) {
  // TODO : implement
  SOF_UNUSED(board);
  SOF_UNUSED(moves);
  SOF_UNUSED(count);
  return ApiResult::NotSupported;
}

ApiResult Engine::stopSearch() {
  // TODO : implement
  return ApiResult::NotSupported;
}

ApiResult Engine::setBool(const std::string &, bool) { return ApiResult::Ok; }
ApiResult Engine::setEnum(const std::string &, size_t) { return ApiResult::Ok; }
ApiResult Engine::setInt(const std::string &, int64_t) { return ApiResult::Ok; }
ApiResult Engine::setString(const std::string &, const std::string &) { return ApiResult::Ok; }
ApiResult Engine::triggerAction(const std::string &) { return ApiResult::Ok; }

}  // namespace SoFSearch
