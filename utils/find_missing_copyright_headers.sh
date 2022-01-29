#!/bin/bash

err=0
for file in $(find $1 -name '*.cc' -or -name '*.h')
do
  read line < "$file"
  if ! [ "${line:0:1}" = "/" ]
   then
    echo $file
    err=1
  fi
done

exit $err
