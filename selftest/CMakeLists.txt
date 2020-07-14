cmake_minimum_required(VERSION 3.8)

project(SoFCheck::SelfTest)

include(Benchmark)

set(CMAKE_CXX_STANDARD 17)


function(add_chess_impl impl_name)
  string(TOUPPER ${impl_name} impl_name_upper)

  add_executable(test_${impl_name}
      test.cpp
      ${ARGN}
  )
  target_compile_definitions(test_${impl_name} PUBLIC INTF_${impl_name_upper})

  add_executable(bench_${impl_name}
    bench.cpp
    ${ARGN}
  )
  target_benchmark(bench_${impl_name})
  target_compile_definitions(bench_${impl_name} PUBLIC INTF_${impl_name_upper})
endfunction()


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
target_link_libraries(bench_sofcheck sof_core sof_util)
