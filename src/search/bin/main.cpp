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

#include <iostream>
#include <string>

#include "bot_api/api_base.h"
#include "bot_api/clients/uci.h"
#include "bot_api/connection.h"
#include "bot_api/connector.h"
#include "bot_api/strutil.h"
#include "core/init.h"
#include "search/search.h"
#include "util/misc.h"
#include "util/result.h"

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
