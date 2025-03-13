#!/bin/bash

set -e

utils_dir="$(realpath "$(dirname "$0")")"

# Clean up the output directory
test -d coverage/json || mkdir -p coverage/json
rm coverage/json/* coverage/report/* >/dev/null 2>&1 || true
cd coverage/json

for unit_test in ../test_runs/*
do
	unit_test="$(basename "$unit_test")"

	for gcda_file_orig_relative in $(find "../test_runs/$unit_test" -name "*.gcda")
	do
		gcda_file_orig="$(realpath "$gcda_file_orig_relative")"
		gcda_file="$(sed 's|#|/|g' <<< "$(basename "$gcda_file_orig")")"
		notes_file="${gcda_file%.gcda}.gcno"

		ln -s -f "$gcda_file_orig" "$gcda_file"
		gcov "$gcda_file" -o "$notes_file" --json-format > /dev/null
		rm "$gcda_file"

		output_basename="$(basename "$gcda_file_orig_relative")"
		output="${output_basename##*#}"
		output="${output%.gcda}.gcov.json.gz"
		mv "$output" "$unit_test-${output_basename%.gcda}.gcov.json.gz"
	done
done

gzip -d *.gz

cd ..
python3 "$utils_dir/code_coverage.py" $@

