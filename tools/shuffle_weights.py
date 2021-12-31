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
import random

parser = argparse.ArgumentParser('Shuffles SoFCheck weights')
parser.add_argument('source_dir', help='source directory of SoFCheck',
                    type=str)
parser.add_argument('binary_dir', help='binary directory of SoFCheck',
                    type=str)
parser.add_argument('-a', '--amplitude', type=int, default=5,
                    help='amplitude of values to be added (default: 5)')
args = parser.parse_args()


def src_path(path):
    return os.path.join(*([args.source_dir] + path))


def bin_path(path):
    return os.path.join(*([args.binary_dir] + path))


dataset_path = src_path(['src', 'eval', 'features.json'])
out = subprocess.check_output([bin_path(['show_weights']), '-f', dataset_path])
weights = list(map(int, out.split()))
weights = \
    [w + random.randint(-args.amplitude, +args.amplitude) for w in weights]
input = ' '.join(map(str, weights))
subprocess.run([bin_path(['apply_weights']), '-f', dataset_path],
               input=input.encode(), check=True)
