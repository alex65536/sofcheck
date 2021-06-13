// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

#include <benchmark/benchmark.h>

// This file contains the main function for benchmarks. We cannot just link against
// `libbenchmark_main`, since we still support `libbenchmark` provided with Ubuntu 18.04 (which
// doesn't contain this static library)

// Ignore the warning about extra semicolon
#pragma GCC diagnostic ignored "-Wpedantic"

BENCHMARK_MAIN();
