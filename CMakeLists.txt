cmake_minimum_required(VERSION 3.8)

project(sofcheck)

add_executable(sofcheck main.cpp)

install(TARGETS sofcheck RUNTIME DESTINATION bin)
