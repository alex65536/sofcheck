cmake_minimum_required(VERSION 3.8)

project(SoFCheck)


# Include helper modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(Benchmark)
include(CheckCXXSymbolExists)
include(CheckCSourceCompiles)


# Declare configuration options
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
  set(USE_BMI1 ON CACHE BOOL "Use BMI1 insruction set (x86_64 only)")
  set(USE_BMI2 OFF CACHE BOOL "Use BMI2 insruction set (x86_64 only)")
endif()


# Detect system configuration
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
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
      "The required headers for BMI2 were not found, so this option is disabled."
    )
    set(USE_BMI2 OFF)
  endif()
endif()

check_cxx_symbol_exists(stpcpy cstring USE_SYSTEM_STPCPY)


# Apply compiler flags
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
  # We assume that the CPU is modern enough (Core and newer) to support SSE4.2 and POPCNT
  add_compile_options(-msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt)
  if(USE_BMI1)
    add_compile_options(-mbmi)
  endif()
  if(USE_BMI2)
    add_compile_options(-mbmi2)
  endif()
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_BINARY_DIR}/src)
include_directories(src)


# Generate files
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/src/core/private)

configure_file(src/config.h.in ${PROJECT_BINARY_DIR}/src/config.h)
add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/src/core/private/near_attacks.h
  COMMAND gen_near_attacks ${PROJECT_BINARY_DIR}/src/core/private/near_attacks.h
  DEPENDS gen_near_attacks
)
add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/src/core/private/magic_consts.h
  COMMAND gen_magic_consts ${PROJECT_BINARY_DIR}/src/core/private/magic_consts.h
  DEPENDS gen_magic_consts
)


# Add targets to build
add_library(sof_util STATIC
  src/util/misc.cpp
  src/util/strutil.cpp
  src/util/random.cpp
)
add_library(sof_core STATIC
  src/core/board.cpp
  src/core/init.cpp
  src/core/move_parser.cpp
  src/core/move.cpp
  src/core/move_chain.cpp
  src/core/movegen.cpp
  src/core/strutil.cpp
  src/core/private/magic.cpp
  src/core/private/zobrist.cpp
  src/core/test/selftest.cpp
  ${PROJECT_BINARY_DIR}/src/core/private/near_attacks.h
  ${PROJECT_BINARY_DIR}/src/core/private/magic_consts.h
)

add_executable(gen_near_attacks
  gen/gen_near_attacks.cpp
  gen/gen_main.cpp
  gen/common.cpp
)
add_executable(gen_magic_consts
  gen/gen_magic_consts.cpp
  gen/gen_main.cpp
  gen/common.cpp
)

add_executable(sofcheck
  src/main.cpp
)
target_link_libraries(sofcheck sof_core sof_util)


# Add benchmarks
add_executable(bench_is_move_legal bench/core/bench_is_move_legal.cpp)
target_benchmark(bench_is_move_legal)
target_link_libraries(bench_is_move_legal sof_core sof_util)

add_executable(bench_move_chain bench/core/bench_move_chain.cpp)
target_benchmark(bench_move_chain)
target_link_libraries(bench_move_chain sof_core sof_util)


# Add self-tests
add_subdirectory(selftest)


# Add install section
install(TARGETS sofcheck RUNTIME DESTINATION bin)
