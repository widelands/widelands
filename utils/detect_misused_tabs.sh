#! /bin/sh
for i in $(find src -name *.h -o -name *.cc);
	do egrep --with-filename --line-number "[^	]+	" $i; done
