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

#ifndef SOF_BOT_API_API_BASE_INCLUDED
#define SOF_BOT_API_API_BASE_INCLUDED

namespace SoFBotApi {

// Result of client or server API calls. This is only the error codes, the error messages (if any)
// must be reported separately via `reportError()`
enum class ApiResult {
  // API call was successful
  Ok,
  // API call is not implemented
  NotSupported,
  // Invalid arguments passed to API call
  InvalidArgument,
  // API methods are called in invalid order (e.g. `stop()` without `search...()` before it)
  UnexpectedCall,
  // Error due to bug in the code which implements this API
  ApiError,
  // Input/output error
  IOError,
  // Unknown error not listed here. More details may be passed with an error message
  RuntimeError
};

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_API_BASE_INCLUDED
