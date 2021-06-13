# Detects `jsoncpp`. If it's present, then the following variable are set:
# - `JSONCPP_TARGET` is set to name of the detected `jsoncpp` target

include_guard(GLOBAL)


# Declare configuration options
set(USE_BUILTIN_JSONCPP OFF CACHE BOOL "Use jsoncpp version located in third-party/")
set(FORCE_EXTERNAL_JSONCPP OFF CACHE BOOL
  "Do not try to use Jsoncpp located in third-party/, raise error instead")


# Detect jsoncpp itself
set(JSONCPP_TARGET "NOTFOUND")

if (NOT USE_BUILTIN_JSONCPP)
  find_package(jsoncpp)

  foreach(cur_target jsoncpp_static jsoncpp_lib)
    if (TARGET "${cur_target}")
      set(JSONCPP_TARGET "${cur_target}")
      message(STATUS "Found jsoncpp target: ${JSONCPP_TARGET}")
      break()
    endif()
  endforeach()
endif()

if("${JSONCPP_TARGET}" STREQUAL "NOTFOUND")
  if(FORCE_EXTERNAL_JSONCPP)
    message(FATAL_ERROR "No suitable jsoncpp target was found")
  endif()

  if (NOT USE_BUILTIN_JSONCPP)
    message(WARNING "No suitable jsoncpp target was found; using version from third-party/")
  endif()

  add_library(jsoncpp_builtin STATIC
    third-party/jsoncpp/jsoncpp.cpp
  )
  target_include_directories(jsoncpp_builtin PUBLIC third-party/jsoncpp)

  set(JSONCPP_TARGET jsoncpp_builtin)
  message(STATUS "Found jsoncpp target: ${JSONCPP_TARGET}")
endif()
