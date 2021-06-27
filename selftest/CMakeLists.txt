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

cmake_minimum_required(VERSION 3.10)

project(SoFCheck_SelfTest)


# Set compiler flags
set(CMAKE_CXX_STANDARD 17)

include(Benchmark)
find_package(UnixCommands)


# Add configuration options
set(DEPTH_DUMP_TRACE_CHAINS OFF CACHE BOOL "\
Prints all the move chains while running depthDump test. (the output may be quite large).")

set(DEPTH_DUMP_LARGE ON CACHE BOOL "\
Run depthDump on bigger recursion depths (consumes more time for test).")

set(RUN_SELF_TESTS ON CACHE BOOL "Run ChessIntf::selfTest() while testing.")

configure_file(selftest_config.h.in "${PROJECT_BINARY_DIR}/src/selftest_config.h")
include_directories("${PROJECT_BINARY_DIR}/src")


# Define a function to add ANY chess rules implementation for testing
function(add_chess_impl impl_name)
  string(TOUPPER "${impl_name}" impl_name_upper)

  add_executable("test_${impl_name}"
      test.cpp
      ${ARGN}
  )
  target_compile_definitions("test_${impl_name}" PUBLIC "INTF_${impl_name_upper}")

  if(benchmark_FOUND)
    add_executable("bench_${impl_name}"
      bench.cpp
      ${ARGN}
    )
    target_benchmark("bench_${impl_name}")
    target_include_directories("bench_${impl_name}" PRIVATE "${PROJECT_SOURCE_DIR}/../bench")
    target_compile_definitions("bench_${impl_name}" PUBLIC "INTF_${impl_name_upper}")
  endif()
endfunction()


# Add some implementations
add_chess_impl(dodecahedron
  dodecahedron/bitboard.cpp
  dodecahedron/board.cpp
  dodecahedron/cpputil.cpp
  dodecahedron/movegen.cpp
  dodecahedron/moves.cpp
  dodecahedron/movestr.cpp
)
# Dodecahedron is too old and too hacky, so I don't plan to fix any warnings in it
if(MSVC)
  set(DODECAHEDRON_COMPILE_OPTIONS /W0)
else()
  set(DODECAHEDRON_COMPILE_OPTIONS -Wno-old-style-cast -Wno-zero-as-null-pointer-constant)
endif()
target_compile_options(test_dodecahedron PRIVATE ${DODECAHEDRON_COMPILE_OPTIONS})
if(benchmark_FOUND)
  target_compile_options(bench_dodecahedron PRIVATE ${DODECAHEDRON_COMPILE_OPTIONS})
endif()

add_chess_impl(sofcheck)
target_link_libraries(test_sofcheck sof_core sof_util)
if(benchmark_FOUND)
  target_link_libraries(bench_sofcheck sof_core sof_util)
endif()

enable_testing()

if(BASH)
  add_test(
    NAME self-test
    COMMAND "${CMAKE_COMMAND}" -E env
      "CMAKE_COMMAND=${CMAKE_COMMAND}"
      "TEST_EXECUTABLE_1=$<TARGET_FILE:test_dodecahedron>"
      "TEST_EXECUTABLE_2=$<TARGET_FILE:test_sofcheck>"
      "BOARDS_FILE=${PROJECT_SOURCE_DIR}/boards.fen"
    "${BASH}" "${PROJECT_SOURCE_DIR}/run_self_tests.sh"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  )
endif()
