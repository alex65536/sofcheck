#!/bin/sh
# This file is part of SoFCheck
#
# Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
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

if [ "$#" -lt 5 ]; then
    echo "Usage: $0 COMMUNICATE_SCRIPT TEST_EXECUTABLE IN_FILE OUT_FILE ANS_FILE"
    exit 1
fi

"$1" "$2" <"$3" >"$4"
diff "$5" "$4"
