find_package(benchmark)

add_library(benchmark_main
  bench/benchmark_main.cpp
)
target_link_libraries(benchmark_main benchmark::benchmark)

function(target_benchmark target)
  if(NOT benchmark_FOUND)
    message(SEND_ERROR "Cannot create a benchmark target, as Google Benchmark is not found!")
    return()
  endif()
  target_include_directories(${target} PRIVATE ${PROJECT_SOURCE_DIR}/bench)
  target_link_libraries(${target} benchmark::benchmark benchmark_main)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(${target} pthread)
  endif()
  if(WIN32)
    target_link_libraries(${target} shlwapi.lib)
  endif()
endfunction()
