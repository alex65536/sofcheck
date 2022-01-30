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
include(Benchmark)
include(CheckCXXSymbolExists)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(JsonCpp)
include(CxxOpts)
find_package(Threads REQUIRED)
find_package(PythonInterp 3.6)
find_package(UnixCommands)
find_package(Git)


# Determine configuration options defaults
if(MINGW)
  # LTO is broken in MinGW. See the following bugs:
  # - https://sourceware.org/bugzilla/show_bug.cgi?id=12762
  # - https://sourceware.org/bugzilla/show_bug.cgi?id=13031
  set(USE_LTO_DEFAULT OFF)
else()
  set(USE_LTO_DEFAULT ON)
endif()


# Declare configuration options
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "^(x86_64|amd64|AMD64)$")
  set(USE_BMI1 ON CACHE BOOL "Use BMI1 insruction set (x86_64 only)")
  set(USE_BMI2 OFF CACHE BOOL "Use BMI2 insruction set (x86_64 only)")
endif()
set(USE_SANITIZERS OFF CACHE BOOL "Enable sanitizers")
set(USE_NO_EXCEPTIONS OFF CACHE BOOL "Build without exception support")
set(USE_SEARCH_DIAGNOSTICS OFF CACHE BOOL
  "Build with search diagnostics (the engine becomes slower but checks itself for errors)")
set(USE_LTO ${USE_LTO_DEFAULT} CACHE BOOL
  "Enable link-time optimizations (makes engine faster, but may be broken on some compilers)")


# Detect system configuration
set(CPU_ARCH unknown)
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "^(x86_64|amd64|AMD64)$")
  set(CPU_ARCH amd64)
endif()
message(STATUS "Detected CPU architecture: ${CPU_ARCH}")

if("${CPU_ARCH}" STREQUAL amd64)
  if(MSVC)
    # MSVC doesn't have the flags for BMI1 and BMI2, but each CPU with these instruction sets has
    # AVX and AVX2 respectively. We enable AVX in hope that it won't make anything worse (if you
    # build code for CPUs with BMI, they will surely have AVX), but the compiler might generate
    # better code with newer instruction sets.
    set(BMI1_FLAGS /arch:AVX)
    set(BMI2_FLAGS /arch:AVX2)
  else()
    set(BMI1_FLAGS -mbmi)
    set(BMI2_FLAGS -mbmi2)
  endif()
  set(CMAKE_REQUIRED_FLAGS ${BMI2_FLAGS})
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
else()
  set(HAS_BMI2 OFF)
endif()
if(USE_BMI2 AND NOT HAS_BMI2)
  message(WARNING "BMI2 intrinsics don't compile, so USE_BMI2 is disabled.")
  set(USE_BMI2 OFF)
endif()

if(MSVC)
  set(HAS_LIBATOMIC OFF)
else()
  set(CMAKE_REQUIRED_FLAGS -latomic)
  check_cxx_source_compiles("
      #include <atomic>
      #include <cstdint>

      struct My {
        uint32_t inner;
      };

      std::atomic<My> x;

      int main() {
        My y = x.load(std::memory_order_relaxed);
        y.inner += 1;
        x.store(y, std::memory_order_relaxed);
        return 0;
      }
    "
    HAS_LIBATOMIC
  )
  unset(CMAKE_REQUIRED_FLAGS)
endif()

include(BoostStacktrace)

check_cxx_symbol_exists(stpcpy cstring USE_SYSTEM_STPCPY)

if(USE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_SUPPORT_OUTPUT)
  if(LTO_SUPPORTED)
    message(STATUS "Enabled link-time optimizations")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(STATUS "Link-time optimizations are NOT supported: ${LTO_SUPPORT_OUTPUT}")
  endif()
else()
  message(STATUS "Link-time optimizations are disabled by configuration")
endif()


# Apply compiler flags
if("${CPU_ARCH}" STREQUAL amd64)
  if(NOT MSVC)
    # We assume that the CPU is modern enough (Core or newer) to support SSE4.2 and POPCNT
    add_compile_options(-msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt)
  endif()
  if(USE_BMI1)
    add_compile_options(${BMI1_FLAGS})
  endif()
  if(USE_BMI2)
    add_compile_options(${BMI2_FLAGS})
  endif()
else()
  if(USE_BMI1 OR USE_BMI2)
    message(WARNING "BMI1 and BMI2 are relevant only on amd64 architecture; disabling them.")
  endif()
  set(USE_BMI1 OFF)
  set(USE_BMI2 OFF)
endif()

if(MSVC)
  add_compile_options(/W4 /WX /wd4146 /wd4127 /wd4068)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS _ENABLE_ATOMIC_ALIGNMENT_FIX)
else()
  # It's better to use as many warnings as possible. The flags are mostly taken from the following
  # post: https://habr.com/ru/post/490850. Not that some warnings are not added, as they either not
  # relevant to the project, or are not supported by Clang (and `clang-tidy` as well). Ah, and we
  # don't use `-Wconversion` and `-Wsign-conversion`, since they would break much code and add much
  # pain to development (i. e. `static_cast`'ing any signed integer before using it as index for
  # the vector)
  add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wnon-virtual-dtor -Woverloaded-virtual
    -Wold-style-cast -Wcast-qual -Wsign-promo
    -Wzero-as-null-pointer-constant -Wextra-semi
    -Werror
  )
endif()

if(MSVC AND USE_NO_EXCEPTIONS)
  message(WARNING "USE_NO_EXCEPTIONS is not supported with MSVC; disabling it.")
  set(USE_NO_EXCEPTIONS OFF)
endif()
if(USE_NO_EXCEPTIONS)
  if(USE_BOOST_STACKTRACE)
    message(WARNING
      "USE_NO_EXCEPTIONS conflicts with USE_BOOST_STACKTRACE. Disabling boost::stacktrace support.")
    set(USE_BOOST_STACKTRACE OFF)
    set(BOOST_STACKTRACE_TARGET)
  endif()
  add_compile_options(-fno-exceptions)
  add_compile_definitions(JSON_USE_EXCEPTION=0)
  add_compile_definitions(CXXOPTS_NO_EXCEPTIONS)
endif()

if(HAS_LIBATOMIC)
  set(LIBATOMIC_TARGET atomic)
else()
  set(LIBATOMIC_TARGET)
endif()

if(MSVC AND USE_SANITIZERS)
  message(WARNING "USE_SANITIZERS is not supported with MSVC; disabling it.")
  set(USE_SANITIZERS OFF)
endif()
if(USE_SANITIZERS)
  add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
  add_link_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
endif()

# Makes compilation of `cxxopts` much faster
add_compile_definitions(CXXOPTS_NO_REGEX)

set(CMAKE_CXX_STANDARD 17)

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
    COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/run_smoke_test.py"
      "$<TARGET_FILE:sofcheck>"
  )
endif()


# Add install section
install(TARGETS sofcheck RUNTIME DESTINATION bin)
