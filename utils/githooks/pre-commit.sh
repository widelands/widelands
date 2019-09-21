#!/bin/bash

## This script will run clang-format on changed files.

git diff --name-only | grep ".*\.\(h\|cc\)" | while read line ; do
   echo formatting file: $line
   clang-format $line
done
