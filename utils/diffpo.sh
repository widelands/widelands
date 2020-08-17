#!/bin/bash

while IFS= read -r line
do
  row=($(echo $line | tr "\t" "\n"))

  if [ ${#row[@]} -eq 3 ] ; then
    if [ ${row[0]} -eq 1 -a ${row[1]} -eq 1 ] ; then
      echo "Skip ${row[2]}"
      git checkout ${row[2]}
    else
      echo "Keep ${line}"
    fi
  fi

done < <(git diff --numstat po)
