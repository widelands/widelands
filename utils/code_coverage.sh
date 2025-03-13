#!/bin/sh

set -e

utils_dir="$(realpath "$(dirname "$0")")"

# Clean up the output directory
test -d coverage/json || mkdir -p coverage/json
rm coverage/json/* coverage/report/* >/dev/null 2>&1 || true
cd coverage/json

find ../.. -name "*.gcda" | xargs gcov --json-format > /dev/null
gzip -d *.gz

cd ..
python3 "$utils_dir/code_coverage.py" $@

