# Detects `jsoncpp`. If it's present, then the following variable are set:
# - `JSONCPP_TARGET` is set to name of the detected `jsoncpp` target

include_guard(GLOBAL)

find_package(jsoncpp REQUIRED)

set(JSONCPP_TARGET "NOTFOUND")

foreach(cur_target jsoncpp_static jsoncpp_lib)
  if (TARGET "${cur_target}")
    set(JSONCPP_TARGET "${cur_target}")
    message(STATUS "Found jsoncpp target: ${cur_target}")
    break()
  endif()
endforeach()

if("${JSONCPP_TARGET}" STREQUAL "NOTFOUND")
  message(FATAL_ERROR "No suitable jsoncpp target was found")
endif()
