#! /bin/sh
for i in $(find src -name *.h -o -name *.cc);
	do egrep --color --with-filename --line-number "[^	]+	" $i; done
