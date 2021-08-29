# Self-testing

## What is it?

This is a small framework to verify correctness of chess rules implementations. It is similar to
[Perft][1], but has more functionality and allows SoFCheck to perform additional checks to ensure
that everything is correct.

The implementation of chess rules in SoFCheck is tested against [Dodecahedron][2], my earlier
attempt to create a good chess engine. Since the rules in these engines were implemented
independently and Dodecahedron was tested for correctness itself, the chance of mistake is very
small.

## Testing process

The self-test process looks as follows. For each tested rules implementation, an executable is
built. You can run this executable and pass a file with boards as an argument. The executable will
print some diagnostic information on standard output. If two executables produce the same output,
then the corresponding rules implementations are either correct or have the same bugs. Most
probably both are correct, since we can assume it's hard to catch the same bug on rules
implementations written independently.

The file with boards is located in this directory, named `boards.fen`. You can generate this file
yourself using `SelftestDataCreate` utility in [`sofcheck-engine-tester`][3] repo if you want.

## Building

Self-tests are already built into SoFCheck. They can be run as a simple test in `ctest`. Running
self-tests involves a Bash script now, so this test is disabled on Windows. Note that the binaries
required for self-testing are still built, so you can run it manually (see section
[below](#running-manually)).

## Running manually

Suppose your current working directory is the root of the repo, the build directory is `build` and
you want to test `sofcheck` against `dodecahedron`. You may want to add other implementations to
test and run them instead (see [below](#adding-your-own-engine)).

~~~
$ # build the project
$ ( cd build && make -j )
$ # run test for sofcheck and save output to /tmp/out1.txt
$ build/selftest/test_sofcheck selftest/boards.fen >/tmp/out1.txt
$ # run test for dodecahedron and save output to /tmp/out2.txt
$ build/selftest/test_dodecahedron selftest/boards.fen >/tmp/out2.txt
$ # compare two outputs
$ diff /tmp/out1.txt /tmp/out2.txt >/tmp/dif.txt && echo OK
~~~

If the outputs are similar, you will see `OK` on the standard output after the last command
completes. If not, the differences between the outputs will be written to `/tmp/dif.txt`. You can
inspect this file to debug your implementation.

For Windows, the process is similar, but the commands may differ. I'm lazy to describe the process
for Windows now.

## Adding your own engine

If you want to test your own rules implementation, you can add it here and build self-tests for it.

Suppose your rules implementation is called `rules` and contains two source files `rules1.cpp` and
`rules2.cpp`.

1. Create directory `rules` in `selftest/` and put your source files into it.

2. Create file `selftest/rules/intf.h`, an interface for your engine that is understandable by
self-tests. You can see the example [here][4]. You need to implement the same functions and define
the same types for your implementation.

3. Add the following lines into `selftest/chess_intf.h` before `#ifndef INTF_USED`. Remember to
change `rules` and `RULES` with your implementation name:

    ```cpp
    #ifdef INTF_RULES
    #ifdef INTF_USED
    #error Interface is already included!
    #endif
    #define INTF_USED
    #include "rules/intf.h"
    #endif
    ```

4. Add your implementation into `selftest/CMakeLists.txt`. You also need to adapt it for your
implementation, replacing its name and the names of the source files:

    ```cmake
    add_chess_impl(rules
      rules/rules1.cpp
      rules/rules2.cpp
    )
    ```

    You may also want to disable some compiler warnings for your implementation, as the compiler
    flags used to build SoFCheck are very strict. To do this, you need to add the following lines
    to `selftest/CMakeLists.txt` (again, replace `rules` and `RULES` with your implementation
    name):

    ```cmake
    set(RULES_COMPILE_OPTIONS -Wno-error -Wno-sign-compare -Wno-old-style-cast)
    target_compile_options(test_rules PRIVATE ${RULES_COMPILE_OPTIONS})
    if(benchmark_FOUND)
      target_compile_options(bench_rules PRIVATE ${RULES_COMPILE_OPTIONS})
    endif()
    ```

5. Rebuild the project. The build must complete successfully, and `selftest/test_rules` executable
must appear.

6. Run the self-test against any existing implementation, as described [above](#running-manually).

## Benchmarks

If you have Google Benchmark installed (and detected by CMake), then benchmarks will be built
alongside with self-tests. The benchmark executable is named `selftest/bench_<engine>` where
`<engine>` is the name of rules implementation. You can just run it and see how fast your
implementation is.

[1]: https://www.chessprogramming.org/Perft
[2]: https://github.com/alex65536/dodecahedron
[3]: https://github.com/alex65536/sofcheck-engine-tester/tree/master/selftest_data_create
[4]: https://github.com/alex65536/sofcheck/blob/analysis-improve/selftest/sofcheck/intf.h
