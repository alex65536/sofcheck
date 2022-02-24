#!/bin/bash
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

# This script is used to create the official releases of SoFCheck. It is indended to run under
# GNU/Linux (to be more precise, it is tested only on Debian GNU/Linux, but should work on other
# distros)

set -e

die() {
    echo "$@" >&2
    exit 1
}

# Paths used in this script. You can modify them freely if needed
SRC_DIR="${PWD}"
BUILD_DIR="${PWD}/build"
TOOLCHAIN_DIR="${PWD}/cmake/toolchains"
OUT_DIR="${PWD}/build"

GREEN="$(echo -e '\033[32;1m')"
YELLOW="$(echo -e '\033[33;1m')"
NORM="$(echo -e '\033[0m')"
BOLD="$(echo -e '\033[1m')"

for OS in windows linux; do
    for CPU in old popcnt bmi2; do
        FLAGS=(
            -DCMAKE_BUILD_TYPE=Release
            -DUSE_STATIC_LINK=ON
            -DUSE_NO_EXCEPTIONS=ON
            -DUSE_LTO=ON
            -DCPU_ARCH_LEVEL="$(tr '[:lower:]' '[:upper:]' <<<"${CPU}")"
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
            -DUSE_BUILTIN_CXXOPTS=ON
            -DUSE_BUILTIN_JSONCPP=ON
        )
        EXE_EXT=""
        if [[ "${OS}" == windows ]]; then
            FLAGS+=(
                -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_DIR}/MinGW64Toolchain.cmake"
            )
            EXE_EXT=".exe"
        elif [[ "$OS" == linux ]]; then
            FLAGS+=(
                -DCMAKE_C_COMPILER=/usr/bin/clang
                -DCMAKE_CXX_COMPILER=/usr/bin/clang++
                -DCMAKE_CXX_LINK_FLAGS='-fuse-ld=lld'
            )
        else
            die "Unknown OS: ${OS}"
        fi
        FULLCPU="amd64-${CPU}"
        echo "${GREEN}Building for${NORM} ${BOLD}${FULLCPU}/${OS}${NORM}..."
        echo "${YELLOW}CMake options:${NORM} ${FLAGS[*]}"
        rm -rf "${BUILD_DIR}/build_${FULLCPU}_${OS}"
        mkdir "${BUILD_DIR}/build_${FULLCPU}_${OS}"
        pushd "${BUILD_DIR}/build_${FULLCPU}_${OS}" >/dev/null
        cmake "${SRC_DIR}" "${FLAGS[@]}"
        python3 owl.py build -p
        cp "sofcheck${EXE_EXT}" "${OUT_DIR}/sofcheck_${FULLCPU}_${OS}${EXE_EXT}"
        popd >/dev/null
    done
done
