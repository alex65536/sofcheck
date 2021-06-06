cmake_minimum_required(VERSION 3.10)

project(SoFCheck)


# Include helper modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(Benchmark)
include(CheckCXXSymbolExists)
include(CheckCSourceCompiles)
include(JsonCpp)
find_package(Threads REQUIRED)


# Declare configuration options
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL x86_64)
  set(USE_BMI1 ON CACHE BOOL "Use BMI1 insruction set (x86_64 only)")
  set(USE_BMI2 OFF CACHE BOOL "Use BMI2 insruction set (x86_64 only)")
endif()
set(USE_SANITIZERS OFF CACHE BOOL "Enable sanitizers")
set(USE_NO_EXCEPTIONS OFF CACHE BOOL "Build without exception support")
set(USE_SEARCH_DIAGNOSTICS OFF CACHE BOOL
  "Build with search diagnostics (the engine becomes slower but checks itself for errors)")


# Detect system configuration
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL x86_64)
  set(CMAKE_REQUIRED_FLAGS -mbmi2)
  check_c_source_compiles("
      #include <immintrin.h>
      #include <stdint.h>

      int main() {
        uint64_t a = _pdep_u64(30, 42);
        uint64_t b = _pext_u64(30, 42);
        return 0;
      }
    "
    HAS_BMI2
  )
  unset(CMAKE_REQUIRED_FLAGS)
  if(USE_BMI2 AND NOT HAS_BMI2)
    message(WARNING
      "The required headers for BMI2 were not found, so this option is disabled.")
    set(USE_BMI2 OFF)
  endif()
endif()

include(BoostStacktrace)

check_cxx_symbol_exists(stpcpy cstring USE_SYSTEM_STPCPY)


# Apply compiler flags
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL x86_64)
  # We assume that the CPU is modern enough (Core and newer) to support SSE4.2 and POPCNT
  add_compile_options(-msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt)
  if(USE_BMI1)
    add_compile_options(-mbmi)
  endif()
  if(USE_BMI2)
    add_compile_options(-mbmi2)
  endif()
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if(USE_NO_EXCEPTIONS)
  if(USE_BOOST_STACKTRACE)
    message(WARNING
      "USE_NO_EXCEPTIONS conflicts with USE_BOOST_STACKTRACE. Disabling boost::stacktrace support.")
    set(USE_BOOST_STACKTRACE OFF)
  endif()
  add_compile_options(-fno-exceptions)
  add_compile_definitions(JSON_USE_EXCEPTION=0)
endif()

if(USE_SANITIZERS)
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fsanitize=address -fsanitize=leak -fsanitize=undefined")
endif()

set(CMAKE_CXX_STANDARD 17)

include_directories("${PROJECT_BINARY_DIR}/src")
include_directories(src)


# Add targets to build
add_library(sof_util STATIC
  src/util/formatter.cpp
  src/util/fileutil.cpp
  src/util/logging.cpp
  src/util/misc.cpp
  src/util/parallel.cpp
  src/util/strutil.cpp
  src/util/random.cpp
)
target_link_libraries(sof_util PRIVATE Threads::Threads ${BOOST_STACKTRACE_TARGET})

add_library(sof_core STATIC
  src/core/board.cpp
  src/core/init.cpp
  src/core/move_parser.cpp
  src/core/move.cpp
  src/core/movegen.cpp
  src/core/strutil.cpp
  src/core/private/magic.cpp
  src/core/private/zobrist.cpp
  src/core/test/selftest.cpp
  "${PROJECT_BINARY_DIR}/src/core/private/near_attacks.h"
  "${PROJECT_BINARY_DIR}/src/core/private/magic_consts.h"
)
target_link_libraries(sof_core PUBLIC sof_util)

add_library(sof_bot_api STATIC
  src/bot_api/connection.cpp
  src/bot_api/options.cpp
  src/bot_api/strutil.cpp
)
target_link_libraries(sof_bot_api PUBLIC sof_core sof_util)

add_library(sof_bot_api_clients STATIC
  src/bot_api/clients/private/uci_option_escape.cpp
  src/bot_api/clients/uci.cpp
)
target_link_libraries(sof_bot_api_clients PUBLIC sof_core sof_util sof_bot_api)

add_library(sof_eval STATIC
  src/eval/evaluate.cpp
  "${PROJECT_BINARY_DIR}/src/eval/feature_count.h"
  "${PROJECT_BINARY_DIR}/src/eval/private/metric.h"
  "${PROJECT_BINARY_DIR}/src/eval/private/weights.h"
  "${PROJECT_BINARY_DIR}/src/eval/private/weight_values.h"
)
target_link_libraries(sof_eval PUBLIC sof_core sof_util sof_bot_api)

add_library(sof_eval_feat STATIC
  src/eval/feat/feat.cpp
)
target_link_libraries(sof_eval_feat
  PUBLIC sof_util
  PRIVATE ${JSONCPP_TARGET}
)

add_library(sof_search STATIC
  src/search/search.cpp
  src/search/private/limits.cpp
  src/search/private/job.cpp
  src/search/private/job_runner.cpp
  src/search/private/move_picker.cpp
  src/search/private/transposition_table.cpp
  src/search/private/util.cpp
)
target_link_libraries(sof_search
  PUBLIC sof_bot_api
  PRIVATE Threads::Threads atomic sof_core sof_util sof_eval
)

add_executable(sofcheck
  src/search/bin/main.cpp
)
target_link_libraries(sofcheck sof_bot_api sof_bot_api_clients sof_search)

add_executable(make_dataset
  src/eval/feat/bin/make_dataset.cpp
)
target_link_libraries(make_dataset sof_core sof_util sof_eval sof_eval_feat ${JSONCPP_TARGET})

add_executable(apply_weights
  src/eval/feat/bin/apply_weights.cpp
)
target_link_libraries(apply_weights sof_util sof_eval_feat ${JSONCPP_TARGET})

add_executable(show_weights
  src/eval/feat/bin/show_weights.cpp
)
target_link_libraries(show_weights sof_util sof_eval_feat ${JSONCPP_TARGET})


# Generate files
include(Generators)
include(GitRevision)

configure_file(src/config.h.in "${PROJECT_BINARY_DIR}/src/config.h")
configure_file(src/version.h.in "${PROJECT_BINARY_DIR}/src/version.h" @ONLY)

generate_file(
  gen/core/near_attacks.cpp
  src/core/private/near_attacks.h
)

generate_file(
  gen/core/magic_consts.cpp
  src/core/private/magic_consts.h
)

generate_file_json(
  gen/eval/feature_count.cpp
  src/eval/feature_count.h
  src/eval/features.json
)
target_link_libraries(gen_eval_feature_count sof_eval_feat)

generate_file(
  gen/eval/metric.cpp
  src/eval/private/metric.h
)

generate_file_json(
  gen/eval/weight_values.cpp
  src/eval/private/weight_values.h
  src/eval/features.json
)
target_link_libraries(gen_eval_weight_values sof_eval_feat)

generate_file_json(
  gen/eval/weights.cpp
  src/eval/private/weights.h
  src/eval/features.json
)
target_link_libraries(gen_eval_weights sof_eval_feat)


# Add benchmarks
if(benchmark_FOUND)
  add_executable(bench_is_move_legal bench/core/bench_is_move_legal.cpp)
  target_benchmark(bench_is_move_legal)
  target_link_libraries(bench_is_move_legal sof_core sof_util)
endif()


# Add self-tests
add_subdirectory(selftest)


# Add tests
enable_testing()

find_package(GTest)

add_executable(test_uci_client
  src/bot_api/clients/test/uci.cpp
)
target_link_libraries(test_uci_client sof_core sof_util sof_bot_api_clients)

if(GTest_FOUND)
  include(GoogleTest)

  add_executable(test_eval_unit_test
    src/eval/test/score.cpp
  )
  target_link_libraries(test_eval_unit_test GTest::GTest GTest::Main)
  gtest_add_tests(TARGET test_eval_unit_test)

  add_executable(test_eval_feat_unit_test
    src/eval/feat/test/feat.cpp
  )
  target_link_libraries(test_eval_feat_unit_test
    sof_eval_feat sof_util ${JSONCPP_TARGET} GTest::GTest GTest::Main
  )
  gtest_add_tests(TARGET test_eval_feat_unit_test)

  add_executable(test_util_unit_test
    src/util/test/parallel.cpp
    src/util/test/valarray.cpp
  )
  target_link_libraries(test_util_unit_test sof_util GTest::GTest GTest::Main)
  gtest_add_tests(TARGET test_util_unit_test)
endif()

if(UNIX)
  # TODO : add code to detect the presence of Python in the system
  add_test(
    NAME uci-client-1
    COMMAND "${PROJECT_SOURCE_DIR}/src/bot_api/clients/test/uci_run.sh"
    "${PROJECT_SOURCE_DIR}/tools/communicate_process.py"
    "${PROJECT_BINARY_DIR}/test_uci_client"
    "${PROJECT_SOURCE_DIR}/src/bot_api/clients/test/uci_in1.txt"
    "${PROJECT_BINARY_DIR}/test_uci_client_log1.txt"
    "${PROJECT_SOURCE_DIR}/src/bot_api/clients/test/uci_out1.txt"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  )
  add_test(
    NAME uci-client-2
    COMMAND "${PROJECT_SOURCE_DIR}/src/bot_api/clients/test/uci_run.sh"
    "${PROJECT_SOURCE_DIR}/tools/communicate_process.py"
    "${PROJECT_BINARY_DIR}/test_uci_client"
    "${PROJECT_SOURCE_DIR}/src/bot_api/clients/test/uci_in2.txt"
    "${PROJECT_BINARY_DIR}/test_uci_client_log2.txt"
    "${PROJECT_SOURCE_DIR}/src/bot_api/clients/test/uci_out2.txt"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  )
  add_test(
    NAME smoke
    COMMAND "${PROJECT_SOURCE_DIR}/tools/run_smoke_test.py"
    "${PROJECT_BINARY_DIR}/sofcheck"
  )
endif()


# Add install section
install(TARGETS sofcheck RUNTIME DESTINATION bin)
