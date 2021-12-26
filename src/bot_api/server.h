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

#ifndef SOF_BOT_API_SERVER_INCLUDED
#define SOF_BOT_API_SERVER_INCLUDED

#include <string>

#include "bot_api/api_base.h"
#include "bot_api/types.h"
#include "core/move.h"
#include "util/misc.h"
#include "util/no_copy_move.h"

namespace SoFBotApi {

// TODO : support multi-PV mode
// TODO : support reporting refutations
// TODO : support "info" subcommands: "seldepth", "tbhits", "sbhits", "cpuload", "currline"

class Client;

// The abstract class server API, i.e. the API that GUI provides to the chess engine. This API is
// mostly based on UCI interface.
//
// This API mostly assumes that all its methods receive valid parameter values (i.e. the client
// should perform its own validation before calling the server). Though, the server may perform its
// own validation and throw `ApiResult::UnexpectedCall` or `ApiResult::InvalidArgument` in case of
// errors.
//
// Thread-safety note:
// - if you implement a server, then the API calls don't need to be thread safe. They will be called
// by the client in a synchronized manner. In contrast, the client you are connected to must be
// thread safe.
// - if you implement a server connector, then the API calls must be thread safe. The server you are
// connected to is not thread-safe.
class Server : public virtual SoFUtil::VirtualNoCopy {
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
  ApiResult sendString(const std::string &str) { return sendString(str.c_str()); }

  // Send temporary search result. Call this method only during the search
  virtual ApiResult sendResult(const SearchResult &result) = 0;

  // Send number of nodes currently searched. Call this method only during the search
  virtual ApiResult sendNodeCount(uint64_t nodes) = 0;

  // Send number of hash table hits. Call this method only during the search
  virtual ApiResult sendHashHits(uint64_t hits) = 0;

  // Send permille of hash full. Call this method only during the search
  virtual ApiResult sendHashFull(permille_t hashFull) = 0;

  // Send currently searched move. `moveNumber` starts from 1 and set to zero if it's undefined.
  // Call this method only during the search
  virtual ApiResult sendCurrMove(SoFCore::Move move, size_t moveNumber = 0) = 0;

  // Report the error from the client. This function (like many others) cannot be called before the
  // connection is finished
  virtual ApiResult reportError(const char *message) = 0;
  ApiResult reportError(const std::string &message) { return reportError(message.c_str()); }

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

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_SERVER_INCLUDED
