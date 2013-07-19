#!/bin/sh

#Requirements:
# scan-build/clang (obviously)
# gcc (used on non-Darwin platforms) 

#Beware:
# Running scan-build from llvm 3.2 will list a large number of false positives
# ("Called C++ object pointer is null" and "Dereference of null pointer").
# Running a newer version is recommended.

if ! [ -d build ]; then
  echo "This script should be run from the root directory."
  exit 1
fi

if [ -e /usr/share/clang/scan-build/c++-analyzer ]; then
  #Debian-based
  ANALYZER=/usr/share/clang/scan-build/c++-analyzer
elif [ -e /usr/lib/clang-analyzer/scan-build/c++-analyzer ]; then
  #Arch
  ANALYZER=/usr/lib/clang-analyzer/scan-build/c++-analyzer
else 
  echo "Could not find c++-analyzer on your platform."
  echo "We are currenly only able to locate it on Arch and Debian-based platforms."
  echo "If you know where we could find it, please let us know."
  exit 1
fi

SOURCE_DIR=`pwd`
BUILD_DIR=$SOURCE_DIR/build/scan-build

mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake $SOURCE_DIR -DCMAKE_CXX_COMPILER=$ANALYZER \
 -DCMAKE_BUILD_TYPE=Debug \
 -DWL_PORTABLE=true 

scan-build make

#The output is stored in /tmp, doesn't need the actual build
cd $SOURCE_DIR
rm -r $BUILD_DIR
