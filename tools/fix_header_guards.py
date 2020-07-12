#!/usr/bin/env python3
import os
from pathlib import Path
import sys


def make_header_guard(file_name):
    file_path = file_name.split('.')[0].split(os.path.sep)
    if file_path[0] == 'src':
        file_path[0] = 'sof'
    elif file_path[0] == 'gen':
        file_path[0] = 'sof_gen'
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


for subdir in ['gen', 'src']:
    p = Path('.') / subdir
    for file_name in p.glob('**/*.h'):
        fix_header_guard(file_name)
