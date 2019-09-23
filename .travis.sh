set -ex

# Create build folder.
mkdir build
cd build

case "$1" in
build)
   cmake .. -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DOPTION_BUILD_TRANSLATIONS=$BUILD_TRANSLATIONS -DOPTION_BUILD_WEBSITE_TOOLS=$BUILD_WEBSITE_TOOLS -DOPTION_ASAN="OFF" -DOPTION_BUILD_CODECHECK="OFF"
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
compiler)
   # Just run this to test whether there are any error during compiling.
   # If this job fails we don't need to run on the compile stage.
   cmake .. -DCMAKE_BUILD_TYPE:STRING="Release" -DOPTION_BUILD_TRANSLATIONS="ON" -DOPTION_BUILD_WEBSITE_TOOLS="ON" -DOPTION_ASAN="OFF"
   make -k -j3
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
