# This file is part of SoFCheck
#
# Copyright (c) 2020-2022 Alexander Kernozhitsky and SoFCheck contributors
#
# SoFCheck is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SoFCheck is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

# Exported variables:
# - `CPU_ARCH`: target CPU architecture (`x86`, `amd64` or `unknown`)
# - configuration options (see below)
# - `USE_SYSTEM_STPCPY`: target has `stpcpy` function
# - `LIBATOMIC_TARGET`: target to support atomics. Empty if such target is not required
include_guard(GLOBAL)

include(CheckCXXSymbolExists)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
find_package(Threads REQUIRED)


# Pre-configuration
set(CPU_ARCH unknown)
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "^(x86_64|amd64|AMD64)$")
  set(CPU_ARCH amd64)
elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "^(x86|X86)$")
  set(CPU_ARCH x86)
endif()
message(STATUS "Detected CPU architecture: ${CPU_ARCH}")

set(USE_LTO_DEFAULT ON)
if(MINGW)
  # LTO is broken in MinGW. See the following bugs:
  # - https://sourceware.org/bugzilla/show_bug.cgi?id=12762
  # - https://sourceware.org/bugzilla/show_bug.cgi?id=13031
  set(USE_LTO_DEFAULT OFF)
endif()
if("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
  # We probably don't want ANY optimizations in debug mode
  set(USE_LTO_DEFAULT OFF)
endif()


# Declare configuration options
if("${CPU_ARCH}" STREQUAL amd64)
  set(USE_BMI1 ON CACHE BOOL "Use BMI1 insruction set (x86_64/amd64 only)")
  set(USE_BMI2 OFF CACHE BOOL "Use BMI2 insruction set (x86_64/amd64 only)")
endif()
set(USE_SANITIZERS OFF CACHE BOOL "Enable sanitizers")
set(USE_NO_EXCEPTIONS OFF CACHE BOOL "Build without exception support")
set(USE_LTO ${USE_LTO_DEFAULT} CACHE BOOL
  "Enable link-time optimizations (makes engine faster, but may be broken on some compilers)")
set(USE_STATIC_LINK OFF CACHE BOOL
  "Link all the binaries statically. Experimental option, use with great care")


# Detect system configuration
if("${CPU_ARCH}" STREQUAL amd64)
  if(MSVC)
    # MSVC doesn't have the flags for BMI1 and BMI2, but each CPU with these instruction sets has
    # AVX and AVX2 respectively. We enable AVX in hope that it won't make anything worse (if you
    # build code for CPUs with BMI, they will surely have AVX), but the compiler might generate
    # better code with newer instruction sets.
    set(BMI1_FLAGS /arch:AVX)
    set(BMI2_FLAGS /arch:AVX2)
  else()
    set(BMI1_FLAGS -mbmi)
    set(BMI2_FLAGS -mbmi2)
  endif()
  set(CMAKE_REQUIRED_FLAGS ${BMI2_FLAGS})
  check_c_source_compiles("
      #include <immintrin.h>
      #include <stdint.h>

      int main() {
        uint64_t a = _pdep_u64(30, 42);
        uint64_t b = _pext_u64(30, 42);
        return 0;
      }
    "
    HAS_BMI2
  )
  unset(CMAKE_REQUIRED_FLAGS)
else()
  set(HAS_BMI2 OFF)
endif()
if(USE_BMI2 AND NOT HAS_BMI2)
  message(WARNING "BMI2 intrinsics don't compile, so USE_BMI2 is disabled.")
  set(USE_BMI2 OFF)
endif()

if(MSVC)
  set(HAS_LIBATOMIC OFF)
else()
  set(CMAKE_REQUIRED_FLAGS -latomic)
  check_cxx_source_compiles("
      #include <atomic>
      #include <cstdint>

      struct My {
        uint32_t inner;
      };

      std::atomic<My> x;

      int main() {
        My y = x.load(std::memory_order_relaxed);
        y.inner += 1;
        x.store(y, std::memory_order_relaxed);
        return 0;
      }
    "
    HAS_LIBATOMIC
  )
  unset(CMAKE_REQUIRED_FLAGS)
endif()

check_cxx_symbol_exists(stpcpy cstring USE_SYSTEM_STPCPY)

if(USE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT LTO_SUPPORT_OUTPUT)
  if(LTO_SUPPORTED)
    message(STATUS "Enabled link-time optimizations")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(STATUS "Link-time optimizations are NOT supported: ${LTO_SUPPORT_OUTPUT}")
  endif()
else()
  message(STATUS "Link-time optimizations are disabled by configuration")
endif()


# Apply compiler flags
if("${CPU_ARCH}" STREQUAL amd64)
  if(NOT MSVC)
    # We assume that the CPU is modern enough (Core or newer) to support SSE4.2 and POPCNT
    add_compile_options(-msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mpopcnt)
  endif()
  if(USE_BMI1)
    add_compile_options(${BMI1_FLAGS})
  endif()
  if(USE_BMI2)
    add_compile_options(${BMI2_FLAGS})
  endif()
else()
  if(USE_BMI1 OR USE_BMI2)
    message(WARNING "BMI1 and BMI2 are relevant only on amd64 architecture; disabling them.")
  endif()
  set(USE_BMI1 OFF)
  set(USE_BMI2 OFF)
endif()

if(MSVC)
  add_compile_options(/W4 /WX /wd4146 /wd4127 /wd4068)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS _ENABLE_ATOMIC_ALIGNMENT_FIX)
else()
  # It's better to use as many warnings as possible. The flags are mostly taken from the following
  # post: https://habr.com/ru/post/490850. Not that some warnings are not added, as they either not
  # relevant to the project, or are not supported by Clang (and `clang-tidy` as well). Ah, and we
  # don't use `-Wconversion` and `-Wsign-conversion`, since they would break much code and add much
  # pain to development (i. e. `static_cast`'ing any signed integer before using it as index for
  # the vector)
  add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wnon-virtual-dtor -Woverloaded-virtual
    -Wold-style-cast -Wcast-qual -Wsign-promo
    -Wzero-as-null-pointer-constant -Wextra-semi
    -Werror
  )
endif()

if(MSVC AND USE_NO_EXCEPTIONS)
  message(WARNING "USE_NO_EXCEPTIONS is not supported with MSVC; disabling it.")
  set(USE_NO_EXCEPTIONS OFF)
endif()
if(USE_NO_EXCEPTIONS)
  add_compile_options(-fno-exceptions)
endif()

if(HAS_LIBATOMIC)
  set(LIBATOMIC_TARGET atomic)
else()
  set(LIBATOMIC_TARGET)
endif()

if(MSVC AND USE_SANITIZERS)
  message(WARNING "USE_SANITIZERS is not supported with MSVC; disabling it.")
  set(USE_SANITIZERS OFF)
endif()
if(USE_SANITIZERS)
  add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
  add_link_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
endif()

if(USE_STATIC_LINK)
  if(MSVC)
    # Use static runtime under MSVC
    cmake_policy(GET CMP0091 CMP0091_NEW)
    if("${CMP0091_NEW}" STREQUAL NEW)
      set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
      macro(fix_msvc_flags variable)
        string(REPLACE "/MD" "/MT" "${variable}" "${${variable}}")
      endmacro()
      fix_msvc_flags(CMAKE_C_FLAGS)
      fix_msvc_flags(CMAKE_C_FLAGS_DEBUG)
      fix_msvc_flags(CMAKE_C_FLAGS_RELEASE)
      fix_msvc_flags(CMAKE_C_FLAGS_MINSIZEREL)
      fix_msvc_flags(CMAKE_C_FLAGS_RELWITHDEBINFO)
      fix_msvc_flags(CMAKE_CXX_FLAGS)
      fix_msvc_flags(CMAKE_CXX_FLAGS_DEBUG)
      fix_msvc_flags(CMAKE_CXX_FLAGS_RELEASE)
      fix_msvc_flags(CMAKE_CXX_FLAGS_MINSIZEREL)
      fix_msvc_flags(CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    endif()
  else()
    if(APPLE)
      message(WARNING
        "Cannot use -static flag on macOS, see https://stackoverflow.com/questions/3801011 for "
        "details. We will still try our best to link every other library statically except the "
        "standard library."
      )
    else()
      add_link_options(-static)
    endif()
  endif()
  if(UNIX AND ("${CMAKE_SYSTEM_NAME}" STREQUAL Linux))
    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
      # Thin LTO is broken somehow with `USE_STATIC_LINK`, and linker emits me errors like
      # `error: Failed to link module: Expected at most one ThinLTO module per bitcode file`.
      # Disabling thin LTO and using full LTO instead solves the issue.
      add_compile_options(-flto=full)
    endif()
    # Statically built application with threading support may segfault under GNU/Linux if we
    # won't do this. For more details, see https://stackoverflow.com/questions/35116327.
    target_link_options(Threads::Threads INTERFACE
      -pthread
      -Wl,--whole-archive
        -lrt
        -lpthread
      -Wl,--no-whole-archive
    )
  endif()
endif()

set(CMAKE_CXX_STANDARD 17)
