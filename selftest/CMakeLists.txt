cmake_minimum_required(VERSION 3.10)
project(chesstest)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -Wpedantic -Werror -mbmi2 -msse4 -mpopcnt)

# Switch compilation mode
# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-O3")
# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fsanitize=address -fsanitize=leak -fsanitize=undefined")

# Include libraries
include_directories(${CMAKE_SOURCE_DIR}/benchmark)

function(add_chess_impl impl_name)
    string(TOUPPER ${impl_name} impl_name_upper)

    add_executable(test_${impl_name}
        test.cpp
        ${ARGN}
    )
    target_compile_definitions(test_${impl_name} PUBLIC INTF_${impl_name_upper})

    add_executable(bench_${impl_name}
        bench.cpp
        benchmark/benchmark-all.cc
        benchmark/benchmark_main.cc
        ${ARGN}
    )
    target_compile_definitions(bench_${impl_name} PUBLIC INTF_${impl_name_upper})
    target_link_libraries(bench_${impl_name} pthread)

    # Fix possible CE (Windows only)
    # target_link_libraries(bench_${impl_name} shlwapi.lib)
endfunction()


add_chess_impl(dodecahedron
    dodecahedron/bitboard.cpp
    dodecahedron/board.cpp
    dodecahedron/cpputil.cpp
    dodecahedron/movegen.cpp
    dodecahedron/moves.cpp
    dodecahedron/movestr.cpp
)

add_chess_impl(seagull
    seagull/rules.cpp
    seagull/position_cost.cpp
)

add_chess_impl(sofcheck
    sofcheck/src/core/board.cpp
    sofcheck/src/core/init.cpp
    sofcheck/src/core/movegen.cpp
    sofcheck/src/core/move.cpp
    sofcheck/src/core/strutil.cpp
    sofcheck/src/core/private/magic.cpp
    sofcheck/src/core/private/zobrist.cpp
    sofcheck/src/core/test/selftest.cpp
    sofcheck/src/util/strutil.cpp
)
include_directories(sofcheck/src)
include_directories(sofcheck/generated)
