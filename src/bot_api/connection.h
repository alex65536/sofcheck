// This file is part of SoFCheck
//
// Copyright (c) 2020-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_BOT_API_CONNECTION_INCLUDED
#define SOF_BOT_API_CONNECTION_INCLUDED

#include <memory>
#include <utility>

#include "bot_api/api_base.h"
#include "bot_api/connector.h"
#include "util/result.h"

namespace SoFBotApi {

class Client;
class Server;

// Class that holds the connection between the client and the server
class Connection {
public:
  // Performs a single `poll()` on the client or on the server (depending on the connection type)
  PollResult poll();

  // Polls the client and the server until it shutdown or error. Returns `Ok` if it gets shutdown,
  // otherwise returns the error returned from `poll()`
  PollResult runPollLoop();

  // Creates a client-side connection (i.e. the type of connections used by chess engines). Returns
  // error if one of the sides was unable to connect
  static SoFUtil::Result<Connection, ApiResult> clientSide(std::unique_ptr<Client> client,
                                                           std::unique_ptr<ServerConnector> server);

  // Creates a server-side connection (i.e. the type of connections used by GUIs). Returns error if
  // one of the sides was unable to connect
  static SoFUtil::Result<Connection, ApiResult> serverSide(std::unique_ptr<ClientConnector> client,
                                                           std::unique_ptr<Server> server);

  Connection(Connection &&) = default;
  Connection &operator=(Connection &&) = default;

  ~Connection();

private:
  Connection(std::unique_ptr<Client> client, std::unique_ptr<Server> server, Connector *connector)
      : client_(std::move(client)), server_(std::move(server)), connector_(connector) {}

  std::unique_ptr<Client> client_;
  std::unique_ptr<Server> server_;
  Connector *connector_;  // Equal either to client or to server, won't be freed automatically
};

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_CONNECTION_INCLUDED
