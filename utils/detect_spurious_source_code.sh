#! /bin/sh
for i in $(find src -name *.h -o -name *.cc); do
	egrep --color --with-filename --line-number -f utils/spurious_source_code_regexps $i;
done
