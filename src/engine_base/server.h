#ifndef SOF_ENGINE_BASE_SERVER_INCLUDED
#define SOF_ENGINE_BASE_SERVER_INCLUDED

#include "core/move.h"
#include "engine_base/api_base.h"
#include "engine_base/types.h"
#include "util/misc.h"

namespace SoFEngineBase {

// TODO : support multi-PV mode
// TODO : support reporting refutations
// TODO : support "info" subcommands: "seldepth", "tbhits", "sbhits", "cpuload", "currline"

class Client;

// The abstract class server API, i.e. the API that GUI provides to the chess engine. This API is
// mostly based on UCI interface.
class Server {
public:
  // Returns server name (or empty string if the name is unknown)
  virtual const char *name() const { return ""; }

  // Returns server author (or empty string if the name is unknown)
  virtual const char *author() const { return ""; }

  // Indicate that the client finished the search, reporting the best move to the server. The client
  // must call this method after `stop()`, `disconnect()` (if the search was running at this time)
  // or when it stops the search itself
  //
  // Note that the search is considered finished only if the function returns without errors
  virtual ApiResult finishSearch(SoFCore::Move bestMove) = 0;

  // Send an arbitrary string message to the server
  virtual ApiResult sendString(const char *str) = 0;

  // Send temporary search result. Call this method only during the search
  virtual ApiResult sendResult(const SearchResult &result) = 0;

  // Send number of nodes currently searched. Call this method only during the search
  virtual ApiResult sendNodeCount(uint64_t nodes) = 0;

  // Send permille of hash full. Call this method only during the search
  virtual ApiResult sendHashFull(permille_t hashFull) = 0;

  // Send currently searched move. `moveNumber` starts from 1 and set to zero if it's undefined.
  // Call this method only during the search
  virtual ApiResult sendCurrMove(SoFCore::Move move, size_t moveNumber = 0) = 0;

  // Report the error from the client. This function (like many others) cannot be called before the
  // connection is finished
  virtual ApiResult reportError(const char *message) = 0;

  virtual ~Server() {}

  friend class Connection;

protected:
  // Initialize connection with the client (the example of the client is the chess engine). This
  // function will be called exactly once right after constructor.
  virtual ApiResult connect(Client *client) = 0;

  // Close connection with the server. This function will be called exactly once right before
  // destructor.
  //
  // If the search is running during disconnection, the API implementation must stop it first
  virtual void disconnect() = 0;
};

}  // namespace SoFEngineBase

#endif  // SOF_ENGINE_BASE_SERVER_INCLUDED
