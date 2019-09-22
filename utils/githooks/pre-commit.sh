#!/bin/bash

## This script will run clang-format on changed files.

git diff --cached --name-only --diff-filter=ACM | grep ".*\.\(h\|cc\)" | while read line ; do
   echo formatting file: $line
   clang-format -i $line
done
