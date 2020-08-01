#include <iostream>
#include <string>

#include "bot_api/clients/uci.h"
#include "bot_api/connection.h"
#include "bot_api/strutil.h"
#include "core/init.h"
#include "search/search.h"
#include "util/misc.h"

using SoFBotApi::ApiResult;
using SoFBotApi::Connection;
using SoFBotApi::PollResult;
using SoFUtil::panic;

const char banner[] = R"R(
                 /    ^   ^    \
                /    / 0 0 \    \
               ||    \  v  /    ||
               ||    /     \    ||
               ||   / /   \ \   ||
               ||   \/ \__/\/   ||
                \      |  |     /
                 \     ^  ^    /
   __          ___      __
  /  \        |        /  \  |                |
  \__    __   |__     /      |__    __    __  |
     \  /  \  |       \      |  |  /__\  /    |_/
  \__/  \__/  |        \__/  |  |  \__   \__  | \
)R";

int main(int argc, const char *argv[]) {
  SOF_UNUSED(argc);
  SOF_UNUSED(argv);

  SoFCore::init();

  std::cout << banner << std::endl;

  auto connResult =
      Connection::clientSide<SoFSearch::Engine, SoFBotApi::Clients::UciServerConnector>();
  if (!connResult.isOk()) {
    panic(std::string("Unable to initialize the engine: ") +
          apiResultToStr(connResult.unwrapErr()));
  }
  Connection connection = connResult.unwrap();
  const PollResult res = connection.runPollLoop();
  if (res != PollResult::Ok) {
    panic(std::string("Fatal error while processing commands: ") + pollResultToStr(res));
  }

  return 0;
}
