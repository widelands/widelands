#!/bin/sh

# Script for running clang-tidy in the CI pipeline

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

if ! [ -d build ] ; then
  mkdir build
end

pushd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
python ../utils/run-clang-tidy.py -checks=-*,*braces*,cert*,clang-analyzer*,cppcoreguidelines-pro-type-static-cast-downcast,google*,performance* > ../clang-tidy.log
popd
