#include "engine_base/connection.h"

#include <utility>

namespace SoFEngineBase {

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
    if (result != PollResult::Ok) {
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

}  // namespace SoFEngineBase
