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

# Runs the engine on test positions and ensures that it doesn't crash.
import sys
import subprocess
from subprocess import PIPE

POSITIONS = [
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r1b1k2r/2qnbppp/p2ppn2/1p4B1/3NPPP1/2N2Q2/PPP4P/2KR1B1R w kq - 0 11",
    "1rq1r1k1/1p3ppp/pB3n2/3ppP2/Pbb1P3/1PN2B2/2P2QPP/R1R4K w - - 1 21",
    "4r1k1/3R1ppp/8/5P2/p7/6PP/4pK2/1rN1B3 w - - 4 43",
    "6K1/8/8/1k3q2/3Q4/8/8/8 w - - 0 1",
    "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1",
    "4k3/8/8/pppppppp/PPPPPPPP/8/8/4K3 w - - 0 1",
    "8/PPPPPPPP/8/2k1K3/8/8/pppppppp/8 w - - 0 1",
    "5K2/1N1N1N2/8/1N1N1N2/1n1n1n2/8/1n1n1n2/5k2 w - - 0 1",
    "3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/NR4Q1/kN1BB1K1 w - - 0 1",
    "r3k2r/pp4pp/8/3Pp3/8/8/PP4PP/R3K2R w KQkq e6 0 1",
    "8/8/3k4/8/4Q1K1/8/8/8 w - - 0 1",
    "8/8/3k4/8/4Q1K1/8/8/8 b - - 0 1"
]
MOVETIME = 4000

if len(sys.argv) != 2:
    sys.stderr.write("Usage: {} EXECUTABLE\n".format(sys.argv[0]))
    sys.exit(1)
exe = sys.argv[1]

proc = subprocess.Popen([exe], bufsize=1, stdin=PIPE, stdout=PIPE,
                        universal_newlines=True)


def check_engine_death(proc):
    if proc.poll() is not None:
        sys.stderr.write('Engine suddenly died with exit code {}\n'
                         .format(proc.returncode))
        sys.exit(1)


proc.stdin.write('uci\n')
while True:
    line = proc.stdout.readline()
    check_engine_death(proc)
    if line == 'uciok\n':
        break

for position in POSITIONS:
    print('Checking position "' + position + '"...')
    proc.stdin.write('position fen ' + position + '\n')
    proc.stdin.write('go movetime {}\n'.format(MOVETIME))
    while True:
        line = proc.stdout.readline()
        check_engine_death(proc)
        if line.startswith('bestmove '):
            break
proc.stdin.write('quit\n')
exitcode = proc.wait(5.0)
if exitcode != 0:
    sys.stderr.write('Engine exited with exit code {}\n'.format(exitcode))
    sys.exit(1)
