#ifndef SOF_ENGINE_BASE_CONNECTION_INCLUDED
#define SOF_ENGINE_BASE_CONNECTION_INCLUDED

#include <memory>

#include "engine_base/api_base.h"
#include "engine_base/client.h"
#include "engine_base/server.h"
#include "util/result.h"

namespace SoFEngineBase {

// Results of `poll()` calls
enum class PollResult {
  // Success
  Ok,
  // Success, but received shutdown message
  Shutdown,
  // I/O error
  IOError,
  // Unknown error
  RuntimeError
};

// Abstract class that is used to connect to client or server. It can wait until it gets some data
class Connector {
public:
  // Block current thread until the connector gets some data
  virtual PollResult poll() = 0;
};

// Connector that emulates a client. Calls server API
class ClientConnector : public Client, public Connector {};

// Connector that emulates a server. Calls server API
class ServerConnector : public Server, public Connector {};

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

  template <typename Client, typename Server>
  inline static SoFUtil::Result<Connection, ApiResult> clientSide() {
    return clientSide(std::make_unique<Client>(), std::make_unique<Server>());
  }

  template <typename Client, typename Server>
  inline static SoFUtil::Result<Connection, ApiResult> serverSide() {
    return serverSide(std::make_unique<Client>(), std::make_unique<Server>());
  }

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

}  // namespace SoFEngineBase

#endif  // SOF_ENGINE_BASE_CONNECTION_INCLUDED
