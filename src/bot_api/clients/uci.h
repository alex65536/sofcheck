#ifndef SOF_BOT_API_CLIENTS_UCI_INCLUDED
#define SOF_BOT_API_CLIENTS_UCI_INCLUDED

#include <chrono>
#include <istream>
#include <ostream>

#include "bot_api/client.h"
#include "bot_api/connector.h"
#include "bot_api/server.h"
#include "core/move.h"
#include "util/no_copy_move.h"

namespace SoFBotApi::Clients {

// A server connector for UCI chess engines. The main goals of this implementation are conformance
// with the official docs and strict input validation.
//
// To obtain the official UCI documentation, use http://download.shredderchess.com/div/uci.zip.
//
// Currently, the implementation is not fully compliant with the UCI docs, here are the issues:
// - the UCI docs say that there must be no substrings "name" and "value" in "setoption" command.
// This implementation assumes that must be no **tokens** "name" and "value", but such substrings
// are allowed.
// - the UCI docs assume that the options are case-insensitive. This implementation assumes that
// they are case-sensitive.
class UciServerConnector final : public ServerConnector, public SoFUtil::NoCopyMove {
public:
  const char *name() const override { return "UCI Server Connector"; }
  const char *author() const override { return "SoFCheck developers"; }

  ApiResult finishSearch(SoFCore::Move bestMove) override;
  ApiResult sendString(const char *str) override;
  ApiResult sendResult(const SearchResult &result) override;
  ApiResult sendNodeCount(uint64_t nodes) override;
  ApiResult sendHashFull(permille_t hashFull) override;
  ApiResult sendCurrMove(SoFCore::Move move, size_t moveNumber = 0) override;
  ApiResult reportError(const char *message) override;

  PollResult poll() override;

  // Constructs `UciServerConnector` with default streams
  UciServerConnector();

  // Constructs `UciServerConnector` with custom streams
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

  // Lists engine options
  PollResult listOptions();

  // Helper method for `processUciGo`
  PollResult doStartSearch(ApiResult searchStartResult);

  // Processes "setoption" subcommand
  PollResult processUciSetOption(std::istream &tokens);

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

}  // namespace SoFBotApi::Clients

#endif  // SOF_BOT_API_CLIENTS_UCI_INCLUDED
