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

# Detects `cxxopts`. If it's present, then the following variable are set:
# - `CXXOPTS_TARGET` is set to name of the detected `cxxopts` target

include_guard(GLOBAL)

include(Platform)


# Declare configuration options
set(USE_BUILTIN_CXXOPTS OFF CACHE BOOL "Use cxxopts version located in third-party/")
set(FORCE_EXTERNAL_CXXOPTS OFF CACHE BOOL
  "Do not try to use cxxopts located in third-party/, raise error instead")


# Detect cxxopts itself
set(CXXOPTS_TARGET "NOTFOUND")

if (NOT USE_BUILTIN_CXXOPTS)
  find_package(cxxopts 3)

  if (TARGET cxxopts::cxxopts)
    set(CXXOPTS_TARGET cxxopts::cxxopts)
    message(STATUS "Found external cxxopts")
  endif()
endif()

if("${CXXOPTS_TARGET}" STREQUAL "NOTFOUND")
  if(FORCE_EXTERNAL_CXXOPTS)
    message(FATAL_ERROR "No suitable cxxopts target was found")
  endif()

  if (NOT USE_BUILTIN_CXXOPTS)
    message(WARNING "No suitable cxxopts target was found; using version from third-party/")
  endif()

  add_library(cxxopts_builtin INTERFACE)
  target_include_directories(cxxopts_builtin INTERFACE third-party/cxxopts)

  set(CXXOPTS_TARGET cxxopts_builtin)
  message(STATUS "Found cxxopts in third-party/")
endif()


# Add compile options
if(USE_NO_EXCEPTIONS)
  target_compile_definitions("${CXXOPTS_TARGET}" INTERFACE CXXOPTS_NO_EXCEPTIONS)
endif()

# Makes compilation of `cxxopts` much faster
target_compile_definitions("${CXXOPTS_TARGET}" INTERFACE CXXOPTS_NO_REGEX)
