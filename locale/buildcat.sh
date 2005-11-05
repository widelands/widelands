#!/bin/sh

# extract all strings
xgettext -k_ -o widelands.pot ../src/*.cc ../src/*/*.cc ../src/*/*/*.cc

# merge new strings with existing translations
for i in *.po; do
	msgmerge $i widelands.pot > tmp
	mv tmp $i
done

# compile message catalogs
for i in *.po; do
	country=`echo $i | cut -d "." -f 1`
	mkdir -p $country/LC_MESSAGES
	msgfmt -o $country/LC_MESSAGES/widelands.mo $i
done

