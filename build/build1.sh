#!/bin/sh

echo Build creation, step 1
echo
echo Retrieving the latest and greatest from CVS

if [ -d widelands ]; then
   cvs update -dPR
else
   cvs -z3 -d:pserver:anonymous@cvs.widelands.sourceforge.net:/cvsroot/widelands export -D now widelands
fi

echo
echo Creating source and binary package file lists

cat > common.list <<EOF
widelands/ChangeLog
widelands/AUTHORS
widelands/COPYING
widelands/README
EOF

cat > source.list <<EOF
widelands/Makefile
widelands/README-compiling.txt
widelands/README.developers
EOF
find widelands/build ! -type d >> source.list
find widelands/src ! -type d  >> source.list
find widelands/wffcreate ! -type d  >> source.list

find widelands/fonts ! -type d  > artwork.list
find widelands/maps ! -type d  >> artwork.list
find widelands/pics ! -type d  >> artwork.list
find widelands/tribes ! -type d  >> artwork.list
find widelands/worlds ! -type d  >> artwork.list

cat > binary.list <<EOF
widelands/widelands
widelands/widelands.exe
widelands/*.dll
EOF

echo
echo Editing the makefile
$EDITOR widelands/Makefile

echo
echo Compiling the source now

cd widelands
BUILD=release make clean all
cd ..

strip --strip-unneeded widelands/widelands

cat <<EOF

If everything went okay, you've now got the following steps left to do:
 1) Copy widelands.exe and SDL.dll into the widelands directory
 2) Build the packages, e.g:
      tar cjf widelands-XXX-source.tar.bz2 \`cat source.list\`
      zip -r widelands-XXX-binary.zip \`cat binary.list\`
 3) Test the packages

 4) Upload the packages using SourceForge's file system
 5) Announce as news item and on widelands-announce

Have fun! :)

EOF
