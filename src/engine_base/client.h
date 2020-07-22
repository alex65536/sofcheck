#ifndef SOF_ENGINE_BASE_CLIENT_INCLUDED
#define SOF_ENGINE_BASE_CLIENT_INCLUDED

#include <cstdint>
#include <string>

#include "core/board.h"
#include "core/move.h"
#include "engine_base/api_base.h"
#include "engine_base/types.h"
#include "util/misc.h"

namespace SoFEngineBase {

// TODO : add support for engine options
// TODO : add API for ponder
// TODO : add API for "go" subcommands: "searchmoves", "mate"

class Server;

// The abstract class client API, i.e. the API that the chess engine provides to GUI. This API is
// mostly based on UCI interface.
//
// Note that this API is not assumed to be thread-safe.
class Client {
public:
  // Returns engine name
  virtual const char *name() const = 0;

  // Returns engine author
  virtual const char *author() const = 0;

  // Enter debug mode. The engine can report debug info to the GUI
  virtual void enterDebugMode() {}

  // Leave debug mode
  virtual void leaveDebugMode() {}

  // Indicate that the next search will be from a different game. The implementation must not rely
  // on this command much
  virtual ApiResult newGame() { return ApiResult::Ok; }

  // Set the position to analyze. The position is calculated in the following way: start from board
  // `board`, then make `count` moves from the array `moves`, from first to last
  virtual ApiResult setPosition(const SoFCore::Board &board, const SoFCore::Move *moves,
                                size_t count) = 0;

  // The following methods must start search. The search must be done in another thread and don't
  // block the current one.
  //
  // During the search, the position and the options can be changed by API calls, so be careful with
  // this. These changed must not have any effect on the currently running search.
  //
  // The client should send information during the search via `send...()` server calls. Note that
  // the search is considered started only if the `search...()` function returns without errors

  // Infinite search
  virtual ApiResult searchInfinite() = 0;

  // Search on the fixed depth
  virtual ApiResult searchFixedDepth(size_t depth) {
    unused(depth);
    return ApiResult::NotSupported;
  }

  // Search no more than `nodes` nodes
  virtual ApiResult searchFixedNodes(uint64_t nodes) {
    unused(nodes);
    return ApiResult::NotSupported;
  }

  // Search for fixed amount of time
  virtual ApiResult searchFixedTime(milliseconds time) {
    unused(time);
    return ApiResult::NotSupported;
  }

  // Search with given time control
  virtual ApiResult searchTimeControl(const TimeControl &control) = 0;

  // Stop search and report best move via `finishSearch()` server call
  virtual ApiResult stopSearch() = 0;

  // Report the error from the server. This function (like many others) cannot be called before the
  // connection is finished
  virtual ApiResult reportError(const char *message) = 0;
  ApiResult reportError(const std::string &message) { return reportError(message.c_str()); }

  virtual ~Client() {}

  friend class Connection;

protected:
  // Initialize connection with the server (the example of the server is chess GUI). This function
  // will be called exactly once right after constructor.
  virtual ApiResult connect(Server *server) = 0;

  // Close connection with the server. This function will be called exactly once right before
  // destructor.
  //
  // If the search is running during disconnection, the API implementation must stop it first
  virtual void disconnect() = 0;
};

}  // namespace SoFEngineBase

#endif  // SOF_ENGINE_BASE_CLIENT_INCLUDED
