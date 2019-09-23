set -ex

# Create build folder.
mkdir build
cd build

case "$1" in
build)
   if [ "$BUILD_TYPE" == "Debug" ]; then
      # We skip translations and codecheck to speed things up
      cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE" -DOPTION_BUILD_TRANSLATIONS="OFF" -DOPTION_ASAN="OFF" -DOPTION_BUILD_CODECHECK="OFF"
   else
      # We test translations only on release builds, in order to help with job timeouts
      cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE" -DOPTION_BUILD_TRANSLATIONS="ON" -DOPTION_ASAN="OFF"
   fi
   # Do the actual build.
   make -k -j3

   # Run the regression suite only if compiling didn't take too long (to avoid timeouts).
   # On macOS it always fails with a broken GL installation message, so we ommit it.
   if [ "$TRAVIS_OS_NAME" = linux ]; then
       cd ..
       ./regression_test.py -b build/src/widelands
   fi
   ;;
codecheck)
   cmake .. -DCMAKE_BUILD_TYPE:STRING="Debug"
   # Run the codecheck test suite.
   pushd ../cmake/codecheck
   ./run_tests.py
   popd

   # Any codecheck warning is an error. Keep the codebase clean!!
   # Suppress color output.
   TERM=dumb make -j1 codecheck 2>&1 | tee codecheck.out
   if grep '^[/_.a-zA-Z]\+:[0-9]\+:' codecheck.out; then
      echo "You have codecheck warnings (see above) Please fix."
      exit 1 # CodeCheck warnings.
   fi
   ;;
documentation)
   # Any warning is an error.
   pushd ../doc/sphinx
   mkdir source/_static
   ./extract_rst.py
   sphinx-build -W -b json -d build/doctrees source build/json
   popd
   ;;
esac
