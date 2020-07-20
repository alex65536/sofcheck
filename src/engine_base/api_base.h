#ifndef SOF_ENGINE_BASE_API_BASE_INCLUDED
#define SOF_ENGINE_BASE_API_BASE_INCLUDED

namespace SoFEngineBase {

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

}  // namespace SoFEngineBase

#endif  // SOF_ENGINE_BASE_API_BASE_INCLUDED
