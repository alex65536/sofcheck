# This file is part of SoFCheck
#
# Copyright (c) 2022 Alexander Kernozhitsky and SoFCheck contributors
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

# Declare a toolchain for building with MinGW for 64-bit Windows.
# Based on https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/cross_compiling/Mingw

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

# Set compiler prefix. Must be `x86_64-w64-mingw32` for Win64 and `i686-w64-mingw32` for Win32
set(COMPILER_PREFIX "x86_64-w64-mingw32")

# Set paths to compilers
find_program(CMAKE_RC_COMPILER NAMES "${COMPILER_PREFIX}-windres")
find_program(CMAKE_C_COMPILER NAMES "${COMPILER_PREFIX}-gcc-posix")
find_program(CMAKE_CXX_COMPILER NAMES "${COMPILER_PREFIX}-g++-posix")

# Set path to the target environment
set(CMAKE_FIND_ROOT_PATH "/usr/${COMPILER_PREFIX}")

# Adjust the default behaviour of the `find_xxx()` commands: search headers and libraries in the
# target environment, search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
