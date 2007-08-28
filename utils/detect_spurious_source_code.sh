#! /bin/sh
for i in $(find src -name *.h -o -name *.cc); do
	for regexp in " $" "	$" "[^	]+	" " ," ",[^ \"'<]" "[^\"],\"" "[^	 @]\{" "\{ +[^ /\\]" " \}" "\}[^ ,;}\]" "\( " " \)"  "(catch|for|if|switch|while)\(" "[^:alpha:_\"]TRUE[^:alpha:_\"]" "[^:alpha:_\"]FALSE[^:alpha:_\"]" "\(void\)" "(for|if|while) \((/[^/*]|[^/])*\) (/[^/*]|[^/])*[^){};\\]$" "_cast<const( +[_a-zA-Z][_a-zA-Z0-9]*)* *>" "_cast<( *[_a-zA-Z][_a-zA-Z0-9]*)* *\* *const *>"; do
		egrep --color --with-filename --line-number "$regexp" $i;
	done
done
