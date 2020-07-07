cmake_minimum_required(VERSION 3.8)

project(sofcheck)

set(CMAKE_CXX_STANDARD 14)

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(gen_near_attacks gen/gen_near_attacks.cpp)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/core/private)

add_custom_command(
  OUTPUT core/private/near_attacks.h
  COMMAND gen_near_attacks ${PROJECT_BINARY_DIR}/core/private/near_attacks.h
  DEPENDS gen_near_attacks
)
include_directories(${PROJECT_BINARY_DIR})
include_directories(src)

add_executable(sofcheck
  src/main.cpp
  src/core/board.cpp
  ${PROJECT_BINARY_DIR}/core/private/near_attacks.h
)

install(TARGETS sofcheck RUNTIME DESTINATION bin)
