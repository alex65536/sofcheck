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

# Detect current Git revision and put it to the following variables. `${PROJECT_NAME}_GIT_REVISION`
# contains the full commit hash, while `${PROJECT_NAME}_GIT_REVISION_SHORT` contains only the short
# commit hash. If we fail to detect Git revision, these variables are set to `unknown`
set(${PROJECT_NAME}_GIT_REVISION "unknown")
set(${PROJECT_NAME}_GIT_REVISION_SHORT "unknown")

if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
  find_package(Git)
  if(Git_FOUND)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      OUTPUT_VARIABLE ${PROJECT_NAME}_GIT_REVISION
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      OUTPUT_VARIABLE ${PROJECT_NAME}_GIT_REVISION_SHORT
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  endif()
endif()
