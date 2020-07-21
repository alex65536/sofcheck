#ifndef SOF_ENGINE_BASE_CONNECTOR_INCLUDED
#define SOF_ENGINE_BASE_CONNECTOR_INCLUDED

#include "engine_base/client.h"
#include "engine_base/server.h"

namespace SoFEngineBase {

// Results of `poll()` calls
enum class PollResult {
  // Success
  Ok,
  // Success, but no client API commands executed
  NoData,
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

}  // namespace SoFEngineBase

#endif  // SOF_ENGINE_BASE_CONNECTOR_INCLUDED
