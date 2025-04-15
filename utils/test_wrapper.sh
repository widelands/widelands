#/bin/sh

set -e

"$1"  # Run the test

# Rename generated code coverage files
prefix="coverage-$(basename "$1")"

cd coverage/binary

if [ -z "$(find -name "*.gcda")" ]
then
	return  # No coverage files generated
fi

basedir="../test_runs/${prefix}"
! test -d "$basedir" || rm -r "$basedir"
mkdir -p "$basedir"

for file in $(find -name "*.gcda")
do
	dir="$basedir/$(dirname "$file")"
	mkdir -p "$dir"

	mv "$file" "$dir"
done

