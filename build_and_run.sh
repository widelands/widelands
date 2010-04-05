#!/bin/sh
echo " "
echo "#######################################"
echo "# Script to compile and run Widelands #"
echo "#######################################"
echo " "

set -e

if ! test -f src/main.cc ; then
 echo This script must be run from the main directory of the widelands source code
 exit 1
fi

test -d build/compile || mkdir -p build/compile
test -d build/compile/locale || mkdir -p build/compile/locale
test -e locale || ln -s build/compile/locale
cd build/compile

# do not duplicate data directories
# except "po" and "doc" - else some files will be changed in bzr
for i in maps pics tribes worlds campaigns txts ; do
 test -L $i || ( rm -rf $i && ln -s ../../$i )
done

echo "CFLAGS=${CFLAGS} MAKEOPTS=${MAKEOPTS}"

cmake -DWL_PORTABLE=true ../.. -DCMAKE_EXE_CXX_FLAGS=${CFLAGS}
make ${MAKEOPTS}

rm  ../../VERSION || true
rm  ../../widelands || true

mv VERSION ../../VERSION
mv src/widelands ../../widelands

cd ../..
./widelands
