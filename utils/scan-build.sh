#!/bin/sh

#Requirements:
# scan-build/clang (obviously)
# gcc (used on non-Darwin platforms) 

if ! [ -d build ]; then
  echo "This script should be run from the root directory."
  exit 1
fi

SOURCE_DIR=`pwd`
BUILD_DIR=$SOURCE_DIR/build/scan-build

mkdir -p $BUILD_DIR
cd $BUILD_DIR

scan-build cmake $SOURCE_DIR -DCMAKE_BUILD_TYPE=Debug
scan-build make

#The output is stored in /tmp, doesn't need the actual build
cd $SOURCE_DIR
rm -r $BUILD_DIR
