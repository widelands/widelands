#!/bin/bash
DIRECTORY=${1%/}
if [ ! -d $DIRECTORY ]
then
	echo "$DIRECTORY ist kein Verzeichnis"
	exit
fi 
echo "aktuelles Verzeichnis: $DIRECTORY/"
WIDELANDS=$DIRECTORY/Widelands.app/Contents/MacOS/widelands
if [ ! -f $WIDELANDS ]
then
	echo "widelands wurde nicht gefunden"
	exit
fi
find $DIRECTORY -name ".?*" -exec rm -v {} \;
UP=$(dirname $DIRECTORY)
echo "hdiutil create -volname "widelands" -srcfolder "$DIRECTORY" "$UP/widelands.dmg""
hdiutil create -fs HFS+ -volname "widelands" -srcfolder "$DIRECTORY" "$UP/widelands.dmg" 

