#!/usr/bin/env python3
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
