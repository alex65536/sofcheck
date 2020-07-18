#!/bin/sh

set -e

if [ "$#" -lt 4 ]; then
    echo "Usage: $0 TEST_EXECUTABLE_1 TEST_EXECUTABLE_2 BOARDS_FILE REPORT_FILE"
    exit 1
fi

"$1" "$3" >selftest_log1.txt &
"$2" "$3" >selftest_log2.txt
wait
diff selftest_log1.txt selftest_log2.txt >"$4"
