# SoF Check

[![Build Status](https://travis-ci.com/alex65536/sofcheck.svg?branch=master)](https://travis-ci.com/alex65536/sofcheck)

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

## About

SoF Check is a chess engine. As many other chess engines, it uses magic bitboards to generate moves
and alpha-beta pruning to find out how to analyze positions. The main features of SoFCheck are:

- Speed: the critical parts of the program are written to be as fast as possible
- Multithreading: SoF Check uses lazy SMP to perform search in multiple threads simultaneously
- Reusability: the parts of SoF Check can be used as a general-purpose chess library. Also, you can
  relatively easy add new communication protocols to the engine or integrate it into your GUI. Now
  the engine supports only UCI, but the situation will change in the future.

To learn more, see [the project design](DESIGN.md).

## State of the project

The project is in early alpha stage. So the engine doesn't play very strong now, and many useful
heuristics are missing.

## Building from source

The project is using [CMake](https://cmake.org) as a build system. You will also need a
C++17-compatible compiler. The engine is tested on the following compilers:

- GCC >= 8 (GCC 7 and earlier don't support `<charconv>` header)
- Clang >= 8 (Clang 6 and earlier don't support `<charconv>` header, and Clang 7 fails with
  linker error `undefined reference to '__muloti4'` in `std::from_chars`)

Other compilers are not tested.

The following dependencies are optional:
- Google Test (to run tests)
- Google Benchmark (to run benchmarks)
- Boost::Stacktrace (to display stack traces on crash)

To install them all on Debian/Ubuntu, use

~~~~~
$ sudo apt install libgtest-dev libbenchmark-dev libboost-stacktrace-dev
~~~~~

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
$ cmake -DCMAKE_BUILD_TYPE=Release -DUSE_BMI2=ON -DUSE_NO_EXCEPTIONS=ON ..
$ make -j8
~~~~~

For more flags, refer to [CMakeLists.txt](CMakeLists.txt) or use CMake GUI. Note that you should
add `-DUSE_BMI2=ON` only if you have a relatively new Intel CPU which has BMI2 instruction set. Do
not use this flag on AMD CPUs, since it will slow down the engine.

## Running tests

SoFCheck uses CTest. So, you can just invoke `ctest` in `build/` directory after you built the
engine.

## License

**License**: GPL 3+.

_SoF Check_ is free software: you can redistribute it and/or modify it under the terms of the
[GNU General Public License](https://www.gnu.org/licenses/gpl.html) as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any later version.

_SoF Check_ is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
[GNU General Public License](https://www.gnu.org/licenses/gpl.html) for more details.
