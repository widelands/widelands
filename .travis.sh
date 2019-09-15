set -ex

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  #Install requested compiler version for linux

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
  # Upgrade homebrew
  brew update && brew upgrade
  # Install osx dependencies
  # boost, cmake, gettext and icu4c are preinstalled :)
  brew install glew sdl2 sdl2_image sdl2_mixer sdl2_ttf
  # brew doesn't add a link by default
  brew link --force gettext
  # icu4c cannot be forced
  export ICU_ROOT="$(brew --prefix icu4c)"
fi

# Configure the build
mkdir build
cd build

if [ "$BUILD_TYPE" == "Debug" ]; then
   # We test translations only on release builds, in order to help with job timeouts
   # We also skip the website binaries in GCC for debug builds for the same reason
   if [ "$CXX" = "g++" ]; then
     cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE" -DOPTION_BUILD_TRANSLATIONS="OFF" -DOPTION_ASAN="OFF" -DOPTION_BUILD_WEBSITE_TOOLS="OFF"
   else
      cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE" -DOPTION_BUILD_TRANSLATIONS="OFF" -DOPTION_ASAN="OFF"
   fi

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
else
   cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE" -DOPTION_BUILD_TRANSLATIONS="ON" -DOPTION_ASAN="OFF"

   # We test the documentation on release builds to make timeouts for debug builds less likely.
   # Any warning is an error.
   if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
     sudo pip install sphinx
   fi
   if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
     pip2 install sphinx
   fi
   pushd ../doc/sphinx
   mkdir source/_static
   ./extract_rst.py
   sphinx-build -W -b json -d build/doctrees source build/json
   popd
fi

# Do the actual build.
make -k -j3

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  # Run the regression suite. Haven't gotten it working on osx, due to problems with xvfb and/or opengl support.
  cd ..
  ./regression_test.py -b build/src/widelands
fi
