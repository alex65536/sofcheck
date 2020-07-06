cmake_minimum_required(VERSION 3.8)

project(sofcheck)

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(gen_near_attacks gen/gen_near_attacks.cpp)

add_custom_command(
  OUTPUT near_attacks.h
  COMMAND gen_near_attacks ${PROJECT_BINARY_DIR}/near_attacks.h
  DEPENDS gen_near_attacks
)
include_directories(${PROJECT_BINARY_DIR})
include_directories(src)

add_executable(sofcheck
  src/main.cpp
  ${PROJECT_BINARY_DIR}/near_attacks.h
)

install(TARGETS sofcheck RUNTIME DESTINATION bin)
