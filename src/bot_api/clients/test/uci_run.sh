#!/bin/sh

set -e

if [ "$#" -lt 5 ]; then
    echo "Usage: $0 COMMUNICATE_SCRIPT TEST_EXECUTABLE IN_FILE OUT_FILE ANS_FILE"
    exit 1
fi

"$1" "$2" <"$3" >"$4"
diff "$5" "$4"
