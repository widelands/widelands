#!/bin/sh

echo Build creation, step 1
echo
echo Retrieving the latest and greatest from CVS

#cvs -z3 -d:pserver:anonymous@cvs.widelands.sourceforge.net:/cvsroot/widelands export -D now widelands
cvs -z3 -d:pserver:anonymous@cvs.widelands.sourceforge.net:/cvsroot/widelands co -dPR widelands

echo
echo Creating source and binary package file lists

find widelands -maxdepth 1 ! -type d > source.list
cat >> source.list <<EOF
widelands/build/*
widelands/win32/*
EOF
find widelands/src -maxdepth 1 ! -type d >> source.list
find widelands/wffcreate -maxdepth 1 ! -type d >> source.list

cat > binary.list <<EOF
widelands/AUTHORS
widelands/COPYING
widelands/README
widelands/widelands
widelands/widelands.exe
widelands/SDL.dll
widelands/fonts/*
widelands/maps/*
widelands/pics/*
widelands/tribes/*
widelands/worlds/*
EOF

echo
echo Editing the makefile
$EDITOR widelands/Makefile

echo 
echo Compiling the source now

cd widelands
make
cd ..

cat <<EOF

If everything went okay, you've now got the following steps left to do:
 1) Copy widelands.exe and SDL.dll into the widelands directory
 2) Build the packages, e.g:
      tar cjf build-XXX-source.tar.bz2 \`cat source.list\`
      zip -r build-XXX-binary.zip \`cat binary.list\`
 3) Test the packages

 4) Upload the packages using SourceForge's file system
 5) Announce as news item and on widelands-announce

Have fun! :)

EOF
