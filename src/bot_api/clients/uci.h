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

#ifndef SOF_BOT_API_CLIENTS_UCI_INCLUDED
#define SOF_BOT_API_CLIENTS_UCI_INCLUDED

#include <memory>

#include "bot_api/connector.h"

namespace SoFBotApi::Clients {

// Creates a server connector for UCI chess engines. The main goals of this implementation are
// conformance with the official docs and strict input validation.
//
// To obtain the official UCI documentation, use http://download.shredderchess.com/div/uci.zip.
//
// Currently, the implementation is not fully compliant with the UCI docs, here are the issues:
// - the UCI docs say that there must be no substrings "name" and "value" in "setoption" command.
// This implementation assumes that must be no **tokens** "name" and "value", but such substrings
// are allowed.
// - the UCI docs assume that the options are case-insensitive. This implementation assumes that
// they are case-sensitive.
std::unique_ptr<ServerConnector> makeUciServerConnector();
std::unique_ptr<ServerConnector> makeUciServerConnector(std::istream &in, std::ostream &out);

}  // namespace SoFBotApi::Clients

#endif  // SOF_BOT_API_CLIENTS_UCI_INCLUDED
