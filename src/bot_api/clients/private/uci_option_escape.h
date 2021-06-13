// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_BOT_API_CLIENTS_PRIVATE_UCI_OPTION_ESCAPE_INCLUDED
#define SOF_BOT_API_CLIENTS_PRIVATE_UCI_OPTION_ESCAPE_INCLUDED

#include <string>

namespace SoFBotApi::Clients::Private {

// Escapes UCI option name `name`. The UCI documentation requires that the option name cannot
// contain "name" and "value" tokens. `name` must be a list of tokens separated by a single space.
// If not, the function will re-tokenize the string, eliminating extra whitespaces.
std::string uciOptionNameEscape(const std::string &name);

// Unescapes UCI option name `name`
std::string uciOptionNameUnescape(const std::string &name);

// Escapes UCI enumeration item `item`. The UCI documentation requires that the enumeration item
// cannot contain "name", "value" and "val" tokens. `item` must be a list of tokens separated by a
// single space. If not, the function will re-tokenize the string, eliminating extra whitespaces.
std::string uciEnumNameEscape(const std::string &item);

// Unescapes UCI enumeration item `item`
std::string uciEnumNameUnescape(const std::string &item);

}  // namespace SoFBotApi::Clients::Private

#endif  // SOF_BOT_API_CLIENTS_PRIVATE_UCI_OPTION_ESCAPE_INCLUDED
