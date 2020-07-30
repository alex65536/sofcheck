#ifndef SOF_BOT_API_STRUTIL_INCLUDED
#define SOF_BOT_API_STRUTIL_INCLUDED

#include "bot_api/api_base.h"
#include "bot_api/connector.h"

namespace SoFBotApi {

const char *apiResultToStr(ApiResult res);
const char *pollResultToStr(PollResult res);

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_STRUTIL_INCLUDED
