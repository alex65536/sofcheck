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

#include "bot_api/strutil.h"

namespace SoFBotApi {

const char *apiResultToStr(const ApiResult res) {
  switch (res) {
    case ApiResult::Ok:
      return "Ok";
    case ApiResult::NotSupported:
      return "Command not supported";
    case ApiResult::InvalidArgument:
      return "Invalid parameters given";
    case ApiResult::UnexpectedCall:
      return "Unexpected command";
    case ApiResult::ApiError:
      return "Internal error";
    case ApiResult::IOError:
      return "Input/output error";
    case ApiResult::RuntimeError:
      return "Unknown error";
  }
  return "";
}

const char *pollResultToStr(const PollResult res) {
  switch (res) {
    case PollResult::Ok:
      return "Ok";
    case PollResult::NoData:
      return "No data received";
    case PollResult::Shutdown:
      return "Shutdown";
    case PollResult::IOError:
      return "Input/output error";
    case PollResult::RuntimeError:
      return "Unknown error";
  }
  return "";
}

}  // namespace SoFBotApi
