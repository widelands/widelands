echo " "
echo "#######################################"
echo "# Script to compile and run Widelands #"
echo "#######################################"
echo " "

mkdir -p build/compile
cd build/compile
cmake -DWL_INSTALL_PREFIX=. -DWL_INSTALL_DATADIR=. -DWL_INSTALL_BINDIR=. -DWL_INSTALL_LOCALEDIR=locale ../..
make
rm -rf ../../VERSION
rm -rf ../../widelands
rm -rf ../../locale
mv VERSION ../../VERSION
mv src/widelands ../../widelands
mv locale ../../locale

cd ../..
./widelands