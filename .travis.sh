set -ex

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
until sudo apt-get update -qq; do sleep 10; done

if [ "$CXX" = "g++" ]; then 
   sudo apt-get install -qq g++-$GCC_VERSION;
   export CXX="g++-$GCC_VERSION" CC="gcc-$GCC_VERSION";
fi
if [ "$CXX" = "clang++" ]; then
   sudo apt-get install -qq clang-$CLANG_VERSION;
   export CXX="clang++-$CLANG_VERSION" CC="clang-$CLANG_VERSION";
fi

until sudo apt-get install -qq \
   cmake \
   libboost-all-dev \
   libglew-dev \
   libicu-dev \
   libpng-dev \
   libxml2-dev \
   libyajl-dev \
   zlib1g-dev \
   libsdl2-dev \
   libsdl2-image-dev \
   libsdl2-mixer-dev \
   libsdl2-net-dev \
   libsdl2-ttf-dev \
; do sleep 10; done

# Start the actual build. 
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE"
make -k -j1
