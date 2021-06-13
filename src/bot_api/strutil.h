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

#ifndef SOF_BOT_API_STRUTIL_INCLUDED
#define SOF_BOT_API_STRUTIL_INCLUDED

#include "bot_api/api_base.h"
#include "bot_api/connector.h"

namespace SoFBotApi {

const char *apiResultToStr(ApiResult res);
const char *pollResultToStr(PollResult res);

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_STRUTIL_INCLUDED
