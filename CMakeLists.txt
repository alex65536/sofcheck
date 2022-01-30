# This file is part of SoFCheck
#
# Copyright (c) 2020-2022 Alexander Kernozhitsky and SoFCheck contributors
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

cmake_minimum_required(VERSION 3.14)

project(SoFCheck)


# Include helper modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(Platform)
include(Benchmark)
include(BoostStacktrace)
include(JsonCpp)
include(CxxOpts)
include(PGO)
find_package(PythonInterp 3.6)
find_package(UnixCommands)
find_package(Git)


# Declare configuration options
set(USE_SEARCH_DIAGNOSTICS OFF CACHE BOOL
  "Build with search diagnostics (the engine becomes slower but checks itself for errors)")


# Add include directories
include_directories("${PROJECT_BINARY_DIR}/src")
include_directories(src)


# Add targets to build
add_library(sof_version STATIC
  "${PROJECT_BINARY_DIR}/src/version/version.cpp"
)

add_library(sof_util STATIC
  src/util/formatter.cpp
  src/util/ioutil.cpp
  src/util/logging.cpp
  src/util/misc.cpp
  src/util/optparse.cpp
  src/util/parallel.cpp
  src/util/strutil.cpp
  src/util/random.cpp
)
target_link_libraries(sof_util
  PUBLIC ${CXXOPTS_TARGET}
  PRIVATE Threads::Threads ${BOOST_STACKTRACE_TARGET}
)

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

add_library(sof_gameset STATIC
  src/gameset/reader.cpp
  src/gameset/types.cpp
)
target_link_libraries(sof_gameset PUBLIC sof_core sof_util)

add_library(sof_eval STATIC
  src/eval/evaluate.cpp
  "${PROJECT_BINARY_DIR}/src/eval/feature_count.h"
  "${PROJECT_BINARY_DIR}/src/eval/private/bitboard.h"
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
  PRIVATE Threads::Threads ${LIBATOMIC_TARGET} sof_core sof_util sof_eval sof_version
)

add_executable(sofcheck
  src/search/bin/main.cpp
)
target_link_libraries(sofcheck sof_bot_api sof_bot_api_clients sof_search)

add_executable(make_dataset
  src/eval/feat/bin/make_dataset.cpp
)
target_link_libraries(
  make_dataset
  sof_core sof_util sof_eval sof_eval_feat sof_gameset Threads::Threads ${JSONCPP_TARGET}
)

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

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.20)
  configure_file(tools/owl.py.in
    "${PROJECT_BINARY_DIR}/owl.py"
    @ONLY
    FILE_PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE
  )
else()
  configure_file(tools/owl.py.in "${PROJECT_BINARY_DIR}/tmp/owl.py" @ONLY)
  file(COPY
    "${PROJECT_BINARY_DIR}/tmp/owl.py"
    DESTINATION "${PROJECT_BINARY_DIR}"
    FILE_PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE
  )
endif()

configure_file(src/config.h.in "${PROJECT_BINARY_DIR}/src/config.h")

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/version")
if(Git_FOUND AND IS_DIRECTORY "${PROJECT_SOURCE_DIR}/.git")
  set(GIT_HEAD_DEPS "${PROJECT_SOURCE_DIR}/.git/logs/HEAD")
else()
  set(GIT_HEAD_DEPS "")
  set(GIT_EXECUTABLE "")
endif()
add_custom_command(
  OUTPUT "${PROJECT_BINARY_DIR}/src/version/version.cpp"
  COMMAND "${CMAKE_COMMAND}"
    -DINPUT_FILE=${PROJECT_SOURCE_DIR}/src/version/version.cpp.in
    -DOUTPUT_FILE=${PROJECT_BINARY_DIR}/src/version/version.cpp
    -DGIT_EXECUTABLE="${GIT_EXECUTABLE}"
    -P "${PROJECT_SOURCE_DIR}/cmake/scripts/GitRevisionConfig.cmake"
  DEPENDS
    "${PROJECT_SOURCE_DIR}/cmake/scripts/GitRevisionConfig.cmake"
    "${PROJECT_SOURCE_DIR}/src/version/version.cpp.in"
    ${GIT_HEAD_DEPS}
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

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
  gen/eval/bitboard.cpp
  src/eval/private/bitboard.h
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
    src/util/test/strutil.cpp
    src/util/test/valarray.cpp
  )
  target_link_libraries(test_util_unit_test sof_util GTest::GTest GTest::Main)
  gtest_add_tests(TARGET test_util_unit_test)
endif()

if(BASH AND PYTHONINTERP_FOUND)
  foreach(test_id 1 2)
    add_test(
      NAME uci-client-${test_id}
      COMMAND "${CMAKE_COMMAND}" -E env
        "CMAKE_COMMAND=${CMAKE_COMMAND}"
        "PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}"
        "COMMUNICATE_SCRIPT=${PROJECT_SOURCE_DIR}/tools/communicate_process.py"
        "TEST_EXECUTABLE=$<TARGET_FILE:test_uci_client>"
        "IN_FILE=${PROJECT_SOURCE_DIR}/src/bot_api/clients/test/uci_in${test_id}.txt"
        "OUT_FILE=${PROJECT_BINARY_DIR}/test_uci_client_${test_id}.txt"
        "ANS_FILE=${PROJECT_SOURCE_DIR}/src/bot_api/clients/test/uci_out${test_id}.txt"
      "${BASH}" "${PROJECT_SOURCE_DIR}/src/bot_api/clients/test/uci_run.sh"
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    )
  endforeach()
endif()

if(PYTHONINTERP_FOUND)
  add_test(
    NAME smoke
    COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/run_test_positions.py"
      "$<TARGET_FILE:sofcheck>"
  )
endif()


# Add install section
install(TARGETS sofcheck RUNTIME DESTINATION bin)
