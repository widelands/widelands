set -ex

if [ "$CXX" = "g++" ]; then
  sudo apt-get install -qq --force-yes -y g++-$GCC_VERSION;
  export CXX="g++-$GCC_VERSION" CC="gcc-$GCC_VERSION";
fi
if [ "$CXX" = "clang++" ]; then
  sudo apt-get install -qq --force-yes -y clang-$CLANG_VERSION;
  export CXX="clang++-$CLANG_VERSION" CC="clang-$CLANG_VERSION";
fi

# Configure the build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE"

if [ "$BUILD_TYPE" == "Debug" ]; then

   # Build the documentation. Any warning is an error.
   sudo pip install sphinx
   pushd ../doc/sphinx
   mkdir source/_static
   ./extract_rst.py
   sphinx-build -W -b json -d build/doctrees source build/json
   popd

   # Run the codecheck test suite.
   pushd ../cmake/codecheck
   ./run_tests.py
   popd

   # Any codecheck warning is an error in Debug builds. Keep the codebase clean!!
   # Suppress color output.
   TERM=dumb make -j1 codecheck 2>&1 | tee codecheck.out
   if grep '^[/_.a-zA-Z]\+:[0-9]\+:' codecheck.out; then 
      echo "You have codecheck warnings (see above) Please fix."
      exit 1 # CodeCheck warnings.
   fi
fi

# Do the actual build.
make -k -j3

# Run the regression suite.
cd ..
./regression_test.py -b build/src/widelands
