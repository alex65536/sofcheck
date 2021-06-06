cmake_minimum_required(VERSION 3.10)

project(SoFCheck::SelfTest)


# Set compiler flags
set(CMAKE_CXX_STANDARD 17)

include(Benchmark)


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

add_chess_impl(sofcheck)
target_link_libraries(test_sofcheck sof_core sof_util)
if(benchmark_FOUND)
  target_link_libraries(bench_sofcheck sof_core sof_util)
endif()

enable_testing()

if(UNIX)
  # The self-test runner uses diff and shell script, so run only on UNIX-like

  add_test(
    NAME self-test
    COMMAND "${PROJECT_SOURCE_DIR}/run_self_tests.sh"
    "${PROJECT_BINARY_DIR}/test_dodecahedron"
    "${PROJECT_BINARY_DIR}/test_sofcheck"
    "${PROJECT_SOURCE_DIR}/boards.fen"
    "${PROJECT_BINARY_DIR}/report.log"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  )
endif()
