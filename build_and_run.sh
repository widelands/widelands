echo " "
echo "#######################################"
echo "# Script to compile and run Widelands #"
echo "#######################################"
echo " "

mkdir -p build/compile
mkdir -p build/compile/locale
ln -s build/compile/locale
cd build/compile

# do not duplicate data directories
# except "po" and "doc" - else some files will be changed in bzr
rm -rf maps pics tribes worlds campaigns txts
ln -s ../../maps
ln -s ../../pics
ln -s ../../tribes
ln -s ../../worlds
ln -s ../../campaigns
ln -s ../../txts

echo "CFLAGS=${CFLAGS} MAKEOPTS=${MAKEOPTS}"

cmake -DWL_INSTALL_PREFIX=. -DWL_INSTALL_DATADIR=. -DWL_INSTALL_BINDIR=. -DWL_INSTALL_LOCALEDIR=locale -DWL_PORTABLE=true ../.. -DCMAKE_EXE_CXX_FLAGS=${CFLAGS}
make ${MAKEOPTS}

rm -rf ../../VERSION
rm -rf ../../widelands

mv VERSION ../../VERSION
mv src/widelands ../../widelands

cd ../..
./widelands