# SoFCheck 🦉♟

[![Build Status][build-badge]][build-link]

[build-badge]: https://github.com/alex65536/sofcheck/actions/workflows/build.yml/badge.svg?branch=master
[build-link]: https://github.com/alex65536/sofcheck/actions/workflows/build.yml

~~~~~
                 /    ^---^    \
                /    / @ @ \    \
               ||    \  v  /    ||
               ||    /     \    ||
               ||   / /   \ \   ||
               ||   \/\___/\/   ||
                \      | |      /
                 \     ^ ^     /
   __          ___      __
  /  \        |        /  \  |                |
  \__    __   |__     /      |__    __    __  |
     \  /  \  |       \      |  |  /__\  /    |_/
  \__/  \__/  |        \__/  |  |  \__   \__  | \
~~~~~

SoFCheck plays on [CCRL] and [Lichess].

[CCRL]: https://ccrl.chessdom.com/ccrl/4040/cgi/engine_details.cgi?match_length=30&print=Details&each_game=1&eng=SoFCheck%200.9%20beta%2064-bit#SoFCheck_0_9_beta_64-bit
[Lichess]: https://lichess.org/@/SoFCheck

## About

SoFCheck is a chess engine. As many other chess engines, it uses magic bitboards to generate moves
and alpha-beta pruning to find out how to analyze positions. The main features of SoFCheck are:

- Speed: the critical parts of the program are written to be as fast as possible
- Multithreading: SoFCheck uses lazy SMP to perform search in multiple threads simultaneously
- Reusability: the parts of SoFCheck can be used as a general-purpose chess library. Also, you can
  relatively easy add new communication protocols to the engine or integrate it into your GUI. Now
  the engine supports only UCI, but the situation may change in the future
- Measurability: all the changes in the engine are verified to ensure that new version doesn't play
  weaker than the previous one. To achieve this, it's a good idea to run matches between different
  versions of the engine

To learn more, see [the project design](docs/design.md) and [how it works](docs/howitworks.md).

## State of the project

The project is in beta stage. It means that the engine plays strong enough (~2400-2500 Elo,
according to my rough estimation), but there is still much space for improvement.

## Building from source

The project is using [CMake](https://cmake.org) as a build system. You will also need a
C++17-compatible compiler. The engine should build fine with the following compilers:

- GCC >= 8 (GCC 12 is used in CI). GCC <= 7 won't work.
- Clang >= 8 (Clang 14 is used in CI). Clang <= 7 won't work.
- Visual Studio >= 2019 (VS 2019 is used in CI). Visual Studio <= 2017 won't work.

Note that MSVC is not supported as good as GCC and Clang, and the resulting binaries may be slower.
Be aware of it.

Other compilers are not tested.

The following dependencies are optional:
- [_Google Test_](https://github.com/google/googletest/): to run the unit tests
- [_Google Benchmark_](https://github.com/google/benchmark): to run benchmarks
- [`boost::stacktrace`](https://www.boost.org/doc/libs/1_65_0/doc/html/stacktrace.html): to display
stack traces when the application panics
- [`jsoncpp`](https://github.com/open-source-parsers/jsoncpp): to extract evaluation weights
located in a JSON file. This one isn't compiled into the final executable and is used only for
building and tuning the engine. If you don't install jsoncpp, the version located in
`third-party/jsoncpp` will be used
- [`cxxopts`](https://github.com/jarro2783/cxxopts): to parse command line parameters in source
code generators and other development utilities. If you don't install cxxopts, the version located
in `third-party/cxxopts` will be used

To install them all on Debian/Ubuntu, use

~~~~~
$ sudo apt install libgtest-dev libbenchmark-dev libboost-stacktrace-dev libjsoncpp-dev
~~~~~

You may also install `libcxxopts-dev` (version `3.0.0` or later is required), but it's present in
quite new distro releases (Ubuntu 22.04+ and Debian 12+)

To build the engine, do the following:

~~~~~
$ mkdir build
$ cd build
$ cmake ..
$ make -j8
~~~~~

You may also want to build the engine in release mode with maximum optimization. To do this, use

~~~~~
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release -DCPU_ARCH_LEVEL=BMI2 -DUSE_NO_EXCEPTIONS=ON -DUSE_LTO=ON ..
$ make -j8
~~~~~

For more flags, refer to corresponding CMake scripts (like [CMakeLists.txt](CMakeLists.txt) or
[cmake/Platform.cmake](cmake/Platform.cmake)) or use CMake GUI.

Note on `-DCPU_ARCH_LEVEL=BMI2`: it should be used only on modern Intel CPUs and on AMD with
Zen 3 (or later) microarchitecture. Older AMD CPUs have a very slow implementation of `PDEP` and
`PEXT` instructions, so using this flag may slow down the engine greatly.

## Running tests

SoFCheck uses CTest. So, you can just invoke `ctest` in `build/` directory after you built the
engine.

## Thanks to

- [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page), for many useful articles
  and descriptions of various useful heuristics.
- Andrey Brenkman, author of [Ifrit](http://alphagameset.xyz/ifrit/ifrit_chess_engine.html) chess
  engine, for well-commented source code of Ifrit. While looking at these sources, some ideas on
  how to use the heuristics in search became clearer to me.
- [CCRL](https://ccrl.chessdom.com/) testers for testing the engine, and especially Graham Banks
  for awesome tournaments ;)

## License

**License**: GPL 3+.

_SoFCheck_ is free software: you can redistribute it and/or modify it under the terms of the
[GNU General Public License](https://www.gnu.org/licenses/gpl.html) as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any later version.

_SoFCheck_ is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
[GNU General Public License](https://www.gnu.org/licenses/gpl.html) for more details.
