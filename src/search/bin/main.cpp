#include <iostream>
#include <string>

#include "bot_api/clients/uci.h"
#include "bot_api/connection.h"
#include "bot_api/strutil.h"
#include "core/init.h"
#include "search/search.h"
#include "util/misc.h"

using SoFBotApi::Connection;
using SoFBotApi::PollResult;
using SoFUtil::panic;

static const char BANNER[] = R"R(
                 /    ^---^    \
                /    / @ @ \    \
               ||    \  v  /    ||
               ||    /     \    ||
               ||   / /   \ \   ||
               ||   \/\___/\/   ||
                \      | |      /
                 \     ^ ^     /
   __          ___      __
  /  \        |        /  \  |                |
  \__    __   |__     /      |__    __    __  |
     \  /  \  |       \      |  |  /__\  /    |_/
  \__/  \__/  |        \__/  |  |  \__   \__  | \
)R";

int main() {
  SoFCore::init();

  std::cout << BANNER << std::endl;

  Connection connection =
      Connection::clientSide<SoFSearch::Engine, SoFBotApi::Clients::UciServerConnector>().okOrErr(
          [](const auto err) {
            panic(std::string("Unable to initialize the engine: ") +
                  SoFBotApi::apiResultToStr(err));
          });
  const PollResult res = connection.runPollLoop();
  if (res != PollResult::Ok) {
    panic(std::string("Fatal error while processing commands: ") + pollResultToStr(res));
  }

  return 0;
}
