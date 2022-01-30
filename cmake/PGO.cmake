# This file is part of SoFCheck
#
# Copyright (c) 2022 Alexander Kernozhitsky and SoFCheck contributors
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

include_guard(GLOBAL)

set(PROFILE_PATH "${PROJECT_BINARY_DIR}/pgo_data" CACHE PATH "Path to store profile data")
set(PGO_BUILD_TYPE "NONE" CACHE STRING
  "Build type. NONE means that PGO is disabled, TRAIN means generating profiling data, and \
USE means building with the generated data")
set(PGO_BUILD_TYPE_VALUES NONE TRAIN USE)
set_property(CACHE PGO_BUILD_TYPE PROPERTY STRINGS ${PGO_BUILD_TYPE_VALUES})

if(NOT ("${PGO_BUILD_TYPE}" IN_LIST PGO_BUILD_TYPE_VALUES))
  message(FATAL_ERROR "\"${PGO_BUILD_TYPE}\" is an invalid value for PGO_BUILD_TYPE")
endif()

if("${PGO_BUILD_TYPE}" STREQUAL "TRAIN")

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(PGO_TRAIN_FLAGS "-fprofile-generate=${PROFILE_PATH}")
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(PGO_TRAIN_FLAGS "-fprofile-generate=${PROFILE_PATH}")
  else()
    message(FATAL_ERROR "PGO is currently not supported on your platform")
  endif()

  add_compile_options(${PGO_TRAIN_FLAGS})
  add_link_options(${PGO_TRAIN_FLAGS})

elseif("${PGO_BUILD_TYPE}" STREQUAL "USE")

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(PGO_USE_FLAGS "-fprofile-use=${PROFILE_PATH}" -fprofile-correction -Wno-missing-profile)
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(PGO_USE_FLAGS "-fprofile-use=${PROFILE_PATH}")
  else()
    message(FATAL_ERROR "PGO is currently not supported on your platform")
  endif()

  add_compile_options(${PGO_USE_FLAGS})
  add_link_options(${PGO_USE_FLAGS})

endif()
