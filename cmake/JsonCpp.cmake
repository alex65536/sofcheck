# This file is part of SoFCheck
#
# Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
#
# SoFCheck is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SoFCheck is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

# Detects `jsoncpp`. If it's present, then the following variable are set:
# - `JSONCPP_TARGET` is set to name of the detected `jsoncpp` target

include_guard(GLOBAL)

include(Platform)


# Declare configuration options
set(USE_BUILTIN_JSONCPP OFF CACHE BOOL "Use jsoncpp version located in third-party/")
set(FORCE_EXTERNAL_JSONCPP OFF CACHE BOOL
  "Do not try to use jsoncpp located in third-party/, raise error instead")


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


# Add compile options
if(USE_NO_EXCEPTIONS)
  target_compile_definitions("${JSONCPP_TARGET}" INTERFACE JSON_USE_EXCEPTION=0)
endif()
