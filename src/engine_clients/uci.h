#ifndef SOF_ENGINE_CLIENTS_UCI_INCLUDED
#define SOF_ENGINE_CLIENTS_UCI_INCLUDED

#include <chrono>
#include <istream>
#include <ostream>

#include "core/move.h"
#include "engine_base/client.h"
#include "engine_base/connector.h"
#include "engine_base/server.h"
#include "util/no_copy_move.h"

namespace SoFEngineClients {

using SoFEngineBase::ApiResult;
using SoFEngineBase::Client;
using SoFEngineBase::PollResult;

// A server connector for UCI chess engines. It tries to conform the official docs, but still lacks
// some features, because they are not supported by the API now. Such commands are just ignored.
// Another feature of this implementation is careful and strict input validation. The parser tries
// hard to prevent the engine from getting invalid data.
//
// To obtain the official UCI documentation, use http://download.shredderchess.com/div/uci.zip.
class UciServerConnector final : public SoFEngineBase::ServerConnector, public SoFUtil::NoCopyMove {
public:
  const char *name() const override { return "UCI Server Connector"; }
  const char *author() const override { return "SoFCheck developers"; }

  ApiResult finishSearch(SoFCore::Move bestMove) override;
  ApiResult sendString(const char *str) override;
  ApiResult sendResult(const SoFEngineBase::SearchResult &result) override;
  ApiResult sendNodeCount(uint64_t nodes) override;
  ApiResult sendHashFull(SoFEngineBase::permille_t hashFull) override;
  ApiResult sendCurrMove(SoFCore::Move move, size_t moveNumber = 0) override;
  ApiResult reportError(const char *message) override;

  PollResult poll() override;

  // Construct `UciServerConnector` with default streams
  UciServerConnector();

  // Construct `UciServerConnector` with custom
  UciServerConnector(std::istream &in, std::ostream &out, std::ostream &err);

  ~UciServerConnector() override;

protected:
  ApiResult connect(Client *client) override;
  void disconnect() override;

private:
  // Makes sure that the client is connected
  void ensureClient();

  // Reports failures from client side, if any. Returns `result` unchanged
  ApiResult checkClient(ApiResult result);

  // Helper method for `processUciGo`. It runs the logic which is required to start the analysis:
  // checks the API call results, reports the errors to the server, indicates that the search has
  // started.
  //
  // The primary purpose of this method is to reduce copy-paste in `processUciGo` for different
  // `search(...)` methods. Better see the source code for more details.
  PollResult doStartSearch(ApiResult searchStartResult);

  // Processes "position" subcommand
  PollResult processUciPosition(std::istream &tokens);

  // Processes "go" command
  PollResult processUciGo(std::istream &tokens);

  // Tries to interpret the next token on the stream as integral type and put it to `val`. Returns
  // true on success. Otherwise, returns `false`, reports the error and doesn't perform any writes
  // into `val`. `intType` parameter is used for error reporting and denotes reported type name.
  template <typename T>
  bool tryReadInt(T &val, std::istream &stream, const char *intType);

  // Tries to interpret the next token on the stream as milliseconds and put it to `time`. Returns
  // `true` on success. Otherwise returns `false`, reports the error and doesn't perform any writes
  // into `time`.
  bool tryReadMsec(std::chrono::milliseconds &time, std::istream &stream);

  // Returns the amount of time the search is running. If the search was not started, the behaviour
  // is undefined
  inline auto getSearchTime() const { return std::chrono::steady_clock::now() - searchStartTime_; }

  bool searchStarted_;
  bool debugEnabled_;
  std::chrono::time_point<std::chrono::steady_clock> searchStartTime_;
  Client *client_;
  std::istream &in_;
  std::ostream &out_;
  std::ostream &err_;
};

}  // namespace SoFEngineClients

#endif  // SOF_ENGINE_CLIENTS_UCI_INCLUDED
