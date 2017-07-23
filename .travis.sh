set -ex

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  #Install linux dependencies
  exit #FIXME try to short-circuit linux-builds to reduce long build times. I don't really need to know the result of all these builds at this point

  if [ "$CXX" = "g++" ]; then
    sudo apt-get install -qq g++-$GCC_VERSION;
    export CXX="g++-$GCC_VERSION" CC="gcc-$GCC_VERSION";
  fi
  if [ "$CXX" = "clang++" ]; then
    sudo apt-get install -qq clang-$CLANG_VERSION;
    export CXX="clang++-$CLANG_VERSION" CC="clang-$CLANG_VERSION";
  fi
fi

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  # Install osx dependencies
  # boost and cmake are preinstalled :)
  brew install gettext glew icu4c sdl2 sdl2_image sdl2_mixer sdl2_ttf zlib
  # brew doesn't add a link by default
  brew link --force gettext
  brew link --force icu4c
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
#TODO(code review): Fails on osx, probably need to set up xvfb or something similar.
./regression_test.py -b build/src/widelands
