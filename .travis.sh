set -ex

#TODO yes, i'll fix indentation at some point :)
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
#Install linux dependencies
exit #FIXME try to short-circuit linux-builds to reduce long build times. I don't really need to know the result of all these builds at this point

# Some of these commands fail transiently. We keep retrying them until they
# succeed.
if [ "$CXX" = "g++" ]; then
   until sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y; do sleep 10; done
fi
if [ "$CXX" = "clang++" ]; then
  until sudo add-apt-repository "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-$CLANG_VERSION main"; do sleep 10; done;
  wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
fi
until sudo add-apt-repository ppa:zoogie/sdl2-snapshots -y; do sleep 10; done
until sudo apt-get update -qq --force-yes -y; do sleep 10; done

if [ "$CXX" = "g++" ]; then
   sudo apt-get install -qq --force-yes -y g++-$GCC_VERSION;
   export CXX="g++-$GCC_VERSION" CC="gcc-$GCC_VERSION";
fi
if [ "$CXX" = "clang++" ]; then
   sudo apt-get install -qq --force-yes -y clang-$CLANG_VERSION;
   export CXX="clang++-$CLANG_VERSION" CC="clang-$CLANG_VERSION";
fi

until sudo apt-get install -qq --force-yes -y \
   cmake \
   libboost-dev \
   libboost-regex-dev \
   libboost-system-dev \
   libboost-test-dev \
   libglew-dev \
   libicu-dev \
   libpng-dev \
   libsdl2-dev \
   libsdl2-image-dev \
   libsdl2-mixer-dev \
   libsdl2-ttf-dev \
   python-pip \
   zlib1g-dev \
; do sleep 10; done

fi

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
# Install osx dependencies
# boost and cmake are preinstalled :)
brew install glew sdl2 sdl2_image sdl2_mixer sdl2_ttf
fi


# Configure the build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE"

if [ "$BUILD_TYPE" == "Debug" ]; then

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
#FIXME(codereview) Start easy, add complicated things to osx later...
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
fi

# Do the actual build.
make -k -j3

# Run the regression suite.
cd ..
./regression_test.py -b build/src/widelands
