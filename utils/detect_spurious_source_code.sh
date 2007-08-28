#! /bin/sh
for i in $(find src -name *.h -o -name *.cc); do
	for regexp in " $" "	$" "[^	]+	" " ," ",[^ \"'<]" "[^\"],\"" "[^	 @]\{" "\{ +[^ /\\]" " \}" "\}[^ ,;}\]" "\( " " \)"  "if\("  "switch\(" "for\(" "while\(" "catch\(" "[^:alpha:_\"]TRUE[^:alpha:_\"]" "[^:alpha:_\"]FALSE[^:alpha:_\"]" "\(void\)" "if \((/[^/*]|[^/])*\) (/[^/*]|[^/])*[^){};\\]$"; do
		egrep --color --with-filename --line-number "$regexp" $i;
	done
done
