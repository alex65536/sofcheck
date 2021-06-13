# This file is part of SoFCheck
#
# Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

find_package(benchmark)

if(benchmark_FOUND)
  add_library(benchmark_main
    bench/benchmark_main.cpp
  )
  target_link_libraries(benchmark_main benchmark::benchmark)
endif()

function(target_benchmark target)
  if(NOT benchmark_FOUND)
    message(SEND_ERROR "Cannot create a benchmark target, as Google Benchmark is not found!")
    return()
  endif()
  target_include_directories("${target}" PRIVATE "${PROJECT_SOURCE_DIR}/bench")
  target_link_libraries("${target}" benchmark::benchmark benchmark_main)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries("${target}" pthread)
  endif()
  if(WIN32)
    target_link_libraries("${target}" shlwapi.lib)
  endif()
endfunction()
