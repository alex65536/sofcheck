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

#ifndef SOF_SEARCH_SEARCH_INCLUDED
#define SOF_SEARCH_SEARCH_INCLUDED

#include <memory>

#include "bot_api/client.h"

namespace SoFSearch {

// Creates the chess engine. It uses `SoFBotApi::Client` as an interface
std::unique_ptr<SoFBotApi::Client> makeEngine();

}  // namespace SoFSearch

#endif  // SOF_SEARCH_SEARCH_INCLUDED
