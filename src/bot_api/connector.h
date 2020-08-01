#ifndef SOF_BOT_API_CONNECTOR_INCLUDED
#define SOF_BOT_API_CONNECTOR_INCLUDED

#include "bot_api/client.h"
#include "bot_api/server.h"

namespace SoFBotApi {

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
  // Blocks current thread until the connector gets some data. This method is not thread-safe.
  virtual PollResult poll() = 0;
};

// Connector that emulates a client. Calls server API
class ClientConnector : public Client, public Connector {};

// Connector that emulates a server. Calls client API
class ServerConnector : public Server, public Connector {};

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_CONNECTOR_INCLUDED
