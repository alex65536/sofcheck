// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_BOT_API_CLIENT_INCLUDED
#define SOF_BOT_API_CLIENT_INCLUDED

#include <cstdint>
#include <string>

#include "bot_api/api_base.h"
#include "bot_api/types.h"
#include "core/board.h"
#include "core/move.h"
#include "util/no_copy_move.h"

namespace SoFBotApi {

// TODO : add API for ponder
// TODO : add API for "go" subcommands: "searchmoves", "mate"

class Server;
class Options;

// The abstract class client API, i.e. the API that the chess engine provides to GUI. This API is
// mostly based on UCI interface.
//
// This API mostly assumes that all its methods receive valid parameter values (i.e. the server
// should perform its own validation before calling the client). Though, the client may perform its
// own validation and throw `ApiResult::UnexpectedCall` or `ApiResult::InvalidArgument` in case of
// errors.
//
// Thread-safety note:
// - if you implement a client, then the API calls don't need to be thread safe. They will be called
// by the server in a synchronized manner. In contrast, the server you are connected to must be
// thread safe.
// - if you implement a client connector, then the API calls must be thread safe. This rule also
// applies to option storage (use `SyncOptionStorage` to store the options). The server you are
// connected to is not thread-safe.
class Client : public virtual SoFUtil::VirtualNoCopy {
public:
  // Returns engine name
  virtual const char *name() const = 0;

  // Returns engine author
  virtual const char *author() const = 0;

  // Returns engine options
  virtual Options &options() = 0;
  virtual const Options &options() const = 0;

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
  virtual ApiResult searchFixedDepth([[maybe_unused]] size_t depth) {
    return ApiResult::NotSupported;
  }

  // Search no more than `nodes` nodes
  virtual ApiResult searchFixedNodes([[maybe_unused]] uint64_t nodes) {
    return ApiResult::NotSupported;
  }

  // Search for fixed amount of time
  virtual ApiResult searchFixedTime([[maybe_unused]] std::chrono::milliseconds time) {
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

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_CLIENT_INCLUDED
