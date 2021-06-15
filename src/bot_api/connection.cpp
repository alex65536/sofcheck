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

#include "bot_api/connection.h"

#include "bot_api/client.h"
#include "bot_api/server.h"

namespace SoFBotApi {

SoFUtil::Result<Connection, ApiResult> Connection::clientSide(
    std::unique_ptr<Client> client, std::unique_ptr<ServerConnector> server) {
  ApiResult result = client->connect(server.get());
  if (result != ApiResult::Ok) {
    client->disconnect();
    return SoFUtil::Err(result);
  }
  result = server->connect(client.get());
  if (result != ApiResult::Ok) {
    client->disconnect();
    server->disconnect();
    return SoFUtil::Err(result);
  }
  ServerConnector *serverPtr = server.release();
  Connection connection(std::move(client), std::unique_ptr<Server>(serverPtr), serverPtr);
  return SoFUtil::Ok(std::move(connection));
}

SoFUtil::Result<Connection, ApiResult> Connection::serverSide(
    std::unique_ptr<ClientConnector> client, std::unique_ptr<Server> server) {
  ApiResult result = server->connect(client.get());
  if (result != ApiResult::Ok) {
    server->disconnect();
    return SoFUtil::Err(result);
  }
  result = client->connect(server.get());
  if (result != ApiResult::Ok) {
    server->disconnect();
    client->disconnect();
    return SoFUtil::Err(result);
  }
  ClientConnector *clientPtr = client.release();
  Connection connection(std::unique_ptr<Client>(clientPtr), std::move(server), clientPtr);
  return SoFUtil::Ok(std::move(connection));
}

PollResult Connection::poll() { return connector_->poll(); }

PollResult Connection::runPollLoop() {
  for (;;) {
    PollResult result = poll();
    if (result == PollResult::Shutdown) {
      return PollResult::Ok;
    }
    if (result != PollResult::Ok && result != PollResult::NoData) {
      return result;
    }
  }
}

Connection::~Connection() {
  if (client_) {
    client_->disconnect();
  }
  if (server_) {
    server_->disconnect();
  }
}

}  // namespace SoFBotApi
