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

SOURCE_DIR=`pwd`
BUILD_DIR=$SOURCE_DIR/build/scan-build

mkdir -p $BUILD_DIR
cd $BUILD_DIR

scan-build cmake $SOURCE_DIR -DCMAKE_BUILD_TYPE=Debug \
 -DWL_PORTABLE=true 

scan-build make

#The output is stored in /tmp, doesn't need the actual build
cd $SOURCE_DIR
rm -r $BUILD_DIR
