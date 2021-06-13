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

# Communicate with the specified process. Send data to it line by line, wait
# for a short timeout and then read all the available stdout and stderr.
# The communication log is written to stdout.
#
# This script was written to perform baseline testing of UCI subsystem.
import sys
import time
from subprocess import Popen, PIPE
from threading import Thread
from queue import Queue, Empty


def reader(stream, queue):
    while True:
        line = stream.readline()
        if line:
            queue.put(line)
        else:
            return


def print_queue(queue, prefix):
    try:
        while True:
            ln = queue.get_nowait()
            sys.stdout.write(f"{prefix} {ln}")
    except Empty:
        pass


if len(sys.argv) != 2:
    sys.stderr.write("Usage: {} EXECUTABLE\n".format(sys.argv[0]))
    sys.exit(1)

exe = sys.argv[1]
process = Popen([exe], bufsize=1, stdin=PIPE, stdout=PIPE, stderr=PIPE,
                universal_newlines=True)

out_lines = Queue()
t_out = Thread(target=reader, args=(process.stdout, out_lines))
t_out.daemon = True
t_out.start()

err_lines = Queue()
t_err = Thread(target=reader, args=(process.stderr, err_lines))
t_err.daemon = True
t_err.start()

time.sleep(0.060)  # Wait slightly after initialization
for line in sys.stdin:
    process.stdin.write(line)
    process.stdin.flush()
    time.sleep(0.015)
    sys.stdout.write(f"I {line}")
    print_queue(out_lines, "O")
    print_queue(err_lines, "E")

process.stdin.close()
exitcode = process.wait()
sys.stdout.write("M Close\n")
print_queue(out_lines, "O")
print_queue(err_lines, "E")
sys.stdout.write(f"M Exited with exitcode {exitcode}\n")
