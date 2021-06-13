#!/usr/bin/env python3
# This file is part of SoFCheck
#
# Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
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

# Concatenates many files in SoFCheck dataset format (i. e. the input format for make_dataset
# utility), renumerating game IDs to make sure that they are sequential. Note that the script is
# dumb and doesn't check whether given input files are valid
import sys

if len(sys.argv) <= 1:
    sys.stderr.write("Usage: {} INPUT_FILES...\n".format(sys.argv[0]))
    sys.exit(1)
files = sys.argv[1:]

last_id = 0

for cur_name in files:
    cur_file = open(cur_name, 'r')
    for ln in cur_file.readlines():
        ln = ln.strip()
        if not ln.startswith('game'):
            print(ln)
            continue
        n, a, _ = ln.split()
        last_id += 1
        print(n, a, last_id)
