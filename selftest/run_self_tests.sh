#!/bin/bash
# This file is part of SoFCheck
#
# Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
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

set -e

verify_env() {
    local ENV_NAME="$1"
    if [[ -z "${!ENV_NAME}" ]]; then
        echo "${ENV_NAME}" is unset
        exit 1
    fi
}

verify_env CMAKE_COMMAND
verify_env TEST_EXECUTABLE_1
verify_env TEST_EXECUTABLE_2
verify_env BOARDS_FILE

"${TEST_EXECUTABLE_1}" "${BOARDS_FILE}" >selftest_log1.txt &
"${TEST_EXECUTABLE_2}" "${BOARDS_FILE}" >selftest_log2.txt
wait
"${CMAKE_COMMAND}" -E compare_files selftest_log1.txt selftest_log2.txt
