#!/usr/bin/env python3
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
    sys.stderr("Usage: {argv[0]} EXECUTABLE")

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
