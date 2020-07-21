#include "engine_base/strutil.h"

namespace SoFEngineBase {

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

}  // namespace SoFEngineBase
