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

# Detects current Git revision and configures `INPUT_FILE` using this information. The result is
# written to `OUTPUT_FILE`
cmake_minimum_required(VERSION 3.10)

set(INPUT_FILE "" CACHE STRING "Input file name to configure")
set(OUTPUT_FILE "" CACHE STRING "Output file name")
set(GIT_EXECUTABLE "" CACHE STRING "Name of git executable")
if(NOT INPUT_FILE)
  message(FATAL_ERROR "INPUT_FILE was not specified")
endif()
if(NOT OUTPUT_FILE)
  message(FATAL_ERROR "OUTPUT_FILE was not specified")
endif()

set(GIT_REVISION "unknown")
set(GIT_REVISION_SHORT "unknown")
set(GIT_VERSION_NAME "unknown")

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  if(GIT_EXECUTABLE)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE GIT_REVISION
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE GIT_REVISION_SHORT
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" describe --tags --always --match='v*' --dirty
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE GIT_VERSION_NAME
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  else()
    message(WARNING "No git executable found, cannot detect version")
  endif()
else()
  message(WARNING "Not in a git repository, cannot detect version")
endif()

configure_file("${INPUT_FILE}" "${OUTPUT_FILE}" @ONLY)
