#!/usr/bin/env python3
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

import os
from pathlib import Path
import sys


def make_header_guard(file_name):
    file_path = file_name.split('.')[0].split(os.path.sep)
    if file_path[0] == 'src':
        file_path[0] = 'sof'
    else:
        file_path[0] = 'sof_' + file_path[0]
    return '_'.join([s.upper() for s in file_path]) + '_INCLUDED'


def strip_empty_lines(lines):
    while lines and lines[-1].strip() == '':
        lines.pop()
    while lines and lines[0].strip() == '':
        lines.pop()
    return lines


def fix_header_guard(file_name):
    file_name = str(file_name)
    if not file_name.endswith('.h'):
        return
    lines = open(file_name, 'r').readlines()
    strip_empty_lines(lines)
    if not lines:
        sys.stderr.write(f"WARNING: file {file_name} is empty!\n")
        lines = ['\n']
    if lines[0].startswith('// NO_HEADER_GUARD'):
        return
    header_guard = make_header_guard(file_name)
    if len(lines) >= 3 and lines[0].strip().startswith('#ifndef') and \
       lines[1].strip().startswith('#define') and \
       lines[-1].strip().startswith('#endif'):
        # Header guard exists
        pass
    else:
        sys.stderr.write(
            f"WARNING: file {file_name} doesn't have a header guard; " +
            f"creating it.\n")
        lines = ['', ''] + lines + ['']
    lines[0] = f"#ifndef {header_guard}\n"
    lines[1] = f"#define {header_guard}\n"
    lines[-1] = f"#endif  // {header_guard}\n"
    open(file_name, 'w').write(''.join(lines))


for subdir in ['bench', 'gen', 'src']:
    p = Path('.') / subdir
    for file_name in p.glob('**/*.h'):
        fix_header_guard(file_name)
