#!/bin/sh 

./build1.sh

echo 
echo "Cross building for windows 32. Guessing, that everything is installed"
echo "under /usr/local/cross-tools"
echo "For infos about cross compiling check README-compiling.txt in the .. dir"

cp widelands/widelands widelands/widelands.temp
cp widelands/Makefile widelands/Makefile.bak

cat widelands/Makefile.bak | sed 's/^CROSS=NO/CROSS=YES/' > widelands/Makefile

cd widelands 
make clean 
make
cd ..

mv widelands/widelands widelands/widelands.exe
mv widelands/widelands.temp widelands/widelands
mv widelands/Makefile.bak widelands/Makefile

cp /usr/local/cross-tools/i386-mingw32msvc/lib/SDL.dll widelands/
/usr/local/cross-tools/bin/i386-mingw32msvc-strip --strip-unneeded widelands/SDL.dll
/usr/local/cross-tools/bin/i386-mingw32msvc-strip --strip-unneeded widelands/widelands.exe

cat << EOF

Everything is done. you should now be able to pack things into archives!

 1) Build the packages, e.g:
      tar cjf build-XXX-source.tar.bz2 \`cat source.list\`
      zip -r build-XXX-binary.zip \`cat binary.list\`
 2) Test the packages

 3) Upload the packages using SourceForge's file system
 4) Announce as news item and on widelands-announce

Have fun! :)

EOF

