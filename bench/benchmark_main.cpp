#include <benchmark/benchmark.h>

// This file contains the main function for benchmarks. We cannot just link against
// `libbenchmark_main`, since we still support `libbenchmark` provided with Ubuntu 18.04 (which
// doesn't contain this static library)

// Ignore the warning about extra semicolon
#pragma GCC diagnostic ignored "-Wpedantic"

BENCHMARK_MAIN();
