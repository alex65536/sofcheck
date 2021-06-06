# Self-testing

## What is it?

This is a small framework to test correctness of chess rules implementation. It is similar to
[Perft][1], but has more functionality and allows SoFCheck to perform additional checks to ensure
that everything is correct.

The implementation of chess rules in SoFCheck is tested against [Dodecahedron][2], my earlier
attempt to create a good chess engine. Since the rules in these engines were implemented
independently and Dodecahedron was itself tested for correctness, the chance of error is very
small.

## Testing process

The self-test process looks as follows. For each tested rules implementation, an executable is
built. You can run this executable and pass a file with boards as an argument. The executable will
print some diagnostic information on standard output. If two executables produce the same output,
then the corresponding rules implementations are either correct or have the same bugs. Most
probably both are correct, since we can assume that it's hard to catch the same bug on rules
implementations written independently.

The file with boards is located in this directory, names `boards.fen`. You can generate if yourself
using `SelftestDataCreate` utility in [`sofcheck-engine tester` repo][3] if you want.

## Building

Self-tests are already built into SoFCheck. They can be run as a simple test in `ctest`. Running
self-tests involves a Bash script now, so this test is disabled on Windows. Note that the binaries
required for self-testing are still built, so you can run it manually (see section
[below](#running-manually)).

## Running manually

Suppose your current working directory is the root of the repo, the build directory is `build` and
you want to test `sofcheck` against `dodecahedron`. You may want to add other engines to test and
run them instead (see [below](#adding-your-own-engine)).

~~~
$ ( cd build && make -j )  # build the project
$ build/test_sofcheck >/tmp/out1.txt  # run the test for sofcheck and save its output
$ build/test_dodecahedron >/tmp/out2.txt  # run the test for dodecahedron and save its output
$ diff /tmp/out1.txt /tmp/out2.txt >/tmp/dif.txt && echo OK  # compare two outputs
~~~

If the outputs are similar, you will see `OK` on the standard output after the last command. If
not, the differences between the outputs will be written to `/tmp/dif.txt`. You can inspect this
file to debug your implementation.

For Windows, the process is similar, but the commands may differ. I'm lazy to describe the process
for Windows now.

## Adding your own engine

If you want to test your own rules implementation, you can add it here and built self-tests for it.

Suppose your rules implementation is called `rules` and contains two source files `rules1.cpp` and
`rules2.cpp`.

1. Create directory `rules` in `selftest/` and put your source files into it.

2. Create file `selftest/rules/intf.h`, an interface for your engine that is understandable by
self-tests. You can see the example [here][4]. You need to implement the same functions and define
the same types for your implementation.

3. Add the following lines into `selftest/chess_intf.h` before `#ifndef INTF_USED`. Remember to
change `rules` with your implementation name):

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

5. Rebuild the project. The build must complete successfully, and `test_rules` executable must
appear

6. Run the self-test against any existing implementation, as described [above](#running-manually)

## Benchmarks

If you have Google Benchmark installed (and detected by CMake), then the benchmarks will be built
alongside with self-tests. The benchmark executable is named `bench_<engine>` where `<engine>` is the
name of rules implementation.

[1]: https://www.chessprogramming.org/Perft
[2]: https://github.com/alex65536/dodecahedron
[3]: https://github.com/alex65536/sofcheck-engine-tester/tree/master/selftest_data_create
[4]: https://github.com/alex65536/sofcheck/blob/analysis-improve/selftest/sofcheck/intf.h
