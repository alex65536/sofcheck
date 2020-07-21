#ifndef SOF_ENGINE_BASE_STRUTIL_INCLUDED
#define SOF_ENGINE_BASE_STRUTIL_INCLUDED

#include "engine_base/api_base.h"
#include "engine_base/connector.h"

namespace SoFEngineBase {

const char *apiResultToStr(ApiResult res);
const char *pollResultToStr(PollResult res);

}  // namespace SoFEngineBase

#endif  // SOF_ENGINE_BASE_STRUTIL_INCLUDED
