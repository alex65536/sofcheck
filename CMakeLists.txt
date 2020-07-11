cmake_minimum_required(VERSION 3.8)

project(sofcheck)

# The engine will only work for x86_64, so check it here explicitly
if(NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
  message(FATAL_ERROR
    "Cannot build for ${CMAKE_SYSTEM_PROCESSOR}.\n"
    "Currently, only x86_64 is supported."
  )
endif()

find_package(benchmark REQUIRED)

function(target_benchmark target)
  target_link_libraries(${target} benchmark::benchmark)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(${target} pthread)
  endif()
  if(WIN32)
    target_link_libraries(${target} shlwapi.lib)
  endif()
endfunction()

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -Wpedantic -Werror -mbmi2 -msse4 -mpopcnt)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/core/private)

add_custom_command(
  OUTPUT core/private/near_attacks.h
  COMMAND gen_near_attacks ${PROJECT_BINARY_DIR}/core/private/near_attacks.h
  DEPENDS gen_near_attacks
)
add_custom_command(
  OUTPUT core/private/part_lines.h
  COMMAND gen_part_lines ${PROJECT_BINARY_DIR}/core/private/part_lines.h
  DEPENDS gen_part_lines
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(src)

add_library(sof_util STATIC
  src/util/strutil.cpp
)

add_library(sof_core STATIC
  src/core/board.cpp
  src/core/init.cpp
  src/core/movegen.cpp
  src/core/move.cpp
  src/core/strutil.cpp
  src/core/private/magic.cpp
  ${PROJECT_BINARY_DIR}/core/private/near_attacks.h
  ${PROJECT_BINARY_DIR}/core/private/part_lines.h
)

add_executable(gen_near_attacks
  gen/gen_near_attacks.cpp
)
add_executable(gen_part_lines
  gen/gen_part_lines.cpp
)

add_executable(sofcheck
  src/main.cpp
)
target_link_libraries(sofcheck sof_core sof_util)

install(TARGETS sofcheck RUNTIME DESTINATION bin)
