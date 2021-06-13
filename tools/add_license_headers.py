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

import argparse
import subprocess
import os
import sys
import re

try:
    from tqdm import tqdm
except ImportError:
    def tqdm(iterable):
        return iterable


# Project name
PROJECT_NAME = 'SoFCheck'

# License header to insert
LICENSE_HEADER = f'This file is part of {PROJECT_NAME}'
LICENSE_TEXT = f'''\
{PROJECT_NAME} is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

{PROJECT_NAME} is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with {PROJECT_NAME}.  If not, see <https://www.gnu.org/licenses/>.
'''
LICENSE_TEXT_LINES = list(LICENSE_TEXT.strip().split('\n'))

# Regex for copyright line
COPYRIGHT_LINE_REGEX = re.compile(
    '^\\s*Copyright \\(([Cc])\\) ([-,0-9 ]+) (.*)$')

# Only the sources from these paths are checked
WHITELIST = [
    '(bench|cmake|gen|selftest|src|tools)/.*',
    'CMakeLists.txt'
]
WHITELIST_REGEX = [re.compile('^' + pat + '$') for pat in WHITELIST]

# The sources from these paths are additionally excluded
BLACKLIST = [
    'selftest/dodecahedron/(?!intf.h)',
    'tools/add_license_header.py'  # TODO remove
]
BLACKLIST_REGEX = [re.compile('^' + pat + '$') for pat in BLACKLIST]

# Detect file type based on extension
FILETYPE_MATCHES = {
    'cmake': 'cmake',
    'cpp': 'cpp',
    'h': 'cpp',
    'py': 'py',
    'sh': 'sh'
}

# Copyright holder substituted for git commits
COPYRIGHT_HOLDER = 'Alexander Kernozhitsky and SoFCheck contributors'

# Commit IDs to ignore while counting the years
COMMITS_TO_IGNORE = {
    '72ee56278effccbfa1ce2e933d5d32406d5ef73b'
}


class CopyrightLine:
    def __format_years(self):
        years = sorted(self.years)
        right = 0
        year_strs = []
        while right < len(years):
            left = right
            while right < len(years) and \
                    years[right] == years[left] + right - left:
                right += 1
            year_strs.append(
                f'{years[left]}-{years[right - 1]}' if left + 1 != right
                else f'{years[left]}')
        return ', '.join(year_strs)

    def __str__(self):
        return self.to_string()

    def __repr__(self):
        return f'CopyrightLine(author={repr(self.author)}, ' + \
            f'years={repr(self.years)}, copy_symbol={repr(self.copy_symbol)})'

    @staticmethod
    def __parse_years(line):
        result = set()
        for item in line.split(','):
            item = item.strip()
            if item.count('-') == 0:
                result.add(int(item))
                continue
            left, right = map(int, item.split('-'))
            if not (left <= right <= left + 1000) or left < 1970:
                raise RuntimeError('Invalid date range')
            result |= {x for x in range(left, right + 1)}
        return result

    @classmethod
    def from_string(cls, line):
        matched = COPYRIGHT_LINE_REGEX.match(line)
        if not matched:
            return None
        return CopyrightLine(matched[3], cls.__parse_years(matched[2]),
                             matched[1])

    def to_string(self):
        return 'Copyright (' + self.copy_symbol + ') ' + \
            self.__format_years() + ' ' + self.author

    def __init__(self, author, years, copy_symbol='c'):
        self.author = author
        self.years = set(years)
        self.copy_symbol = copy_symbol


class CopyrightLines:
    def contents(self):
        return self.__contents

    def add(self, line):
        if line.author in self.__mapping:
            self.__mapping[line.author].years |= line.years
            return
        self.__mapping[line.author] = line
        self.__contents.append(line)

    def __init__(self):
        self.__mapping = {}
        self.__contents = []


class LicenseAddError(Exception):
    pass


def git(args):
    res = subprocess.run(['git'] + list(args), capture_output=True,
                         universal_newlines=True, check=True)
    return res.stdout


def is_good_filename(name):
    if all((re.match(pat, name) is None for pat in WHITELIST)):
        return False
    return all((re.match(pat, name) is None for pat in BLACKLIST))


def detect_filetype(name):
    basename = os.path.basename(name)
    if basename.lower() == 'cmakelists.txt':
        return 'cmake'
    ext = list(basename.split('.'))
    if len(ext) <= 1:
        return ''
    if ext[-1] == 'in':
        ext = ext[:-1]
    if len(ext) <= 1:
        return ''
    return FILETYPE_MATCHES.get(ext[-1], '')


def get_comment_style(type_name):
    if type_name in {'cpp'}:
        return '//'
    if type_name in {'cmake', 'py', 'sh'}:
        return '#'
    raise RuntimeError('Cannot get comment style for ' + type_name)


def is_prelude(line):
    return line.startswith('#!')


def parse_years_from_git(file_name, first_commit):
    git_log = git(['log', '--format=%H;%ad;%s', '--follow', '^' + first_commit,
                   'HEAD', '--', file_name]).strip().split('\n')
    years = set()
    for line in git_log:
        commit_id, date, message = line.split(';', 2)
        if commit_id in COMMITS_TO_IGNORE:
            continue
        year = int(date.split()[4])
        years.add(year)
    return years


def add_license_header(file_name, file_type, first_commit):
    c_ln = CopyrightLines()
    c_ln.add(CopyrightLine(
        COPYRIGHT_HOLDER, parse_years_from_git(file_name, first_commit)))

    # Read lines
    lines = [ln.rstrip('\n') for ln in open(file_name, 'r').readlines()]
    lines_orig = lines.copy()
    comment = get_comment_style(file_type)

    # Skip prelude (e. g. shebangs in the start of file)
    file_pos = 0
    while file_pos < len(lines) and is_prelude(lines[file_pos]):
        file_pos += 1

    # Add a license header if it's not present
    if file_pos >= len(lines) or \
            lines[file_pos] != comment + ' ' + LICENSE_HEADER:
        sys.stderr.write(
            f'{file_name} doesn\'t have a license header; it will be added.\n')
        to_add = [LICENSE_HEADER, ''] + LICENSE_TEXT_LINES
        nonempty_line_pos = file_pos
        while nonempty_line_pos < len(lines) and \
                lines[nonempty_line_pos] == '':
            nonempty_line_pos += 1
        lines = lines[:file_pos] + \
            [(comment + ' ' + ln).strip() for ln in to_add] + \
            [''] + lines[nonempty_line_pos:]
    file_pos += 1

    # Parse copyright lines
    copy_start_pos = file_pos
    while True:
        if file_pos > len(lines):
            raise LicenseAddError('Bad license header')
        ln = lines[file_pos].strip()
        file_pos += 1
        if ln == comment:
            continue
        if not ln.startswith(comment + ' '):
            raise LicenseAddError('Bad license header')
        ln = ln[len(comment) + 1:]
        line_to_add = CopyrightLine.from_string(ln)
        if not line_to_add:
            file_pos -= 1
            break
        c_ln.add(line_to_add)

    # Merge copyright lines
    lines = lines[:copy_start_pos] + [comment] + \
        [comment + ' ' + ln.to_string() for ln in c_ln.contents()] + \
        [comment] + lines[file_pos:]

    # Overwrite the file if it's changed
    if lines != lines_orig:
        sys.stderr.write(f'{file_name} is modified; overwriting.\n')
        open(file_name, 'w').write('\n'.join(lines) + '\n')


def locate_git_repository():
    prev_dir = None
    while not os.path.isdir('.git'):
        os.chdir(os.path.pardir)
        cur_dir = os.path.realpath(os.path.curdir)
        if cur_dir == prev_dir:
            raise RuntimeError('Unable to locate git repository')
        prev_dir = cur_dir


def locate_first_commit(hint):
    first_commit = hint
    if hint is None:
        first_commit = git(['rev-list', 'HEAD']).split()[-1]
        if not re.match('^[0-9a-f]+$', first_commit):
            raise RuntimeError('Invalid commit ID to start from')
    return first_commit


def add_all_license_headers(first_commit):
    file_list = []
    for file_name in git(['diff', '--name-only', first_commit]).split('\n'):
        if (not file_name) or (not os.path.isfile(file_name)) or \
                (not is_good_filename(file_name)):
            continue
        file_type = detect_filetype(file_name)
        if not file_type:
            continue
        file_list.append((file_name, file_type))

    for file_name, file_type in tqdm(file_list):
        add_license_header(file_name, file_type, first_commit)


parser = argparse.ArgumentParser(
    description='Add GPLv3 license header to the start of each file.')
parser.add_argument(
    '-f', '--from',
    help='consider only the changes after given commit (default: consider ' +
         'all changes)',
    action='store', type=str, metavar='COMMIT', dest='from_commit',
    default=None)
args = parser.parse_args()

locate_git_repository()
first_commit = locate_first_commit(args.from_commit)
add_all_license_headers(first_commit)
