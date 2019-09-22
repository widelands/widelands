set -ex

# Create build folder.
mkdir build
cd build

case "$1" in
build)
   export START_TIME=$(date +%s)
   
   if [ "$BUILD_TYPE" == "Debug" ]; then
      cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE" -DOPTION_BUILD_TRANSLATIONS="OFF" -DOPTION_ASAN="OFF"
   else
      # We test translations only on release builds, in order to help with job timeouts
      cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE" -DOPTION_BUILD_TRANSLATIONS="ON" -DOPTION_ASAN="OFF"
   fi
   # Do the actual build.
   make -k -j3
   
   export STOP_TIME=$(date +%s)

   # Run the regression suite only if compiling didn't take too long (to avoid timeouts).
   # On macOS it always fails with a broken GL installation message, so we ommit it.
   if [ "$TRAVIS_OS_NAME" = linux ]; then
      if [ "$TRAVIS_COMPILER" = g++ ] && (( STOP_TIME - START_TIME >= 1980 )); then
         echo "Not enough time left, to run the regression suit."
      else
         cd ..
         ./regression_test.py -b build/src/widelands
      fi
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
