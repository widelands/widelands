#!/bin/bash

WL_DIR=$(dirname $(dirname "$0"))
cd "$WL_DIR"
if ! [ -f src/wlapplication.cc ] ; then
   echo "ERROR: Cannot find the main directory of the Widelands source code."
   exit 1
fi

if which pngquant > /dev/null
then
  echo "Running pngquant..."
  tempfile=$(mktemp)

  i=0
  skipped=0
  for image in $(find "data" -name '*.png')
  do
    ((++i))
    printf "\r[%5d] %-100s " "$i" "$image"
    pngquant 256 < "$image" > "$tempfile"
    if [ $(wc -c < "$tempfile") -lt $(wc -c < "$image") ]
    then
      mv "$tempfile" "$image"
    else
      ((++skipped))
    fi
  done
fi

printf "\n%d images converted, %d images skipped.\n\n" $((i-skipped)) $skipped

echo "Running other tools..."
./utils/optimize_pngs.py  # NOCOM
