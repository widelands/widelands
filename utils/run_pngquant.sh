#!/bin/bash

# Optimize images with pngquant.
# Call without arguments to optimize all images.
# Call with arguments to optimize only specified images and images in the specified directories.
# To optimize only changed images, you can use
#   git diff --numstat master data/ | grep -oP "\S+.png$" | xargs utils/run_pngquant.sh

if ! which pngquant > /dev/null
then
  echo "ERROR: pngquant not found"
  exit 1
fi

dirs=()
if [ -z "$1" ]
then
  WL_DIR=$(dirname $(dirname "$0"))
  cd "$WL_DIR"
  if ! [ -f src/wlapplication.cc ]
  then
    echo "ERROR: Cannot find the main directory of the Widelands source code."
    exit 1
  fi
  dirs+=("data")
else
  while [ -n "$1" ]
  do
    dirs+=("$1")
    shift
  done
fi

echo "Running pngquant..."
tempfile="$(mktemp -d)/image.png"

i=0
skipped=0
saved=0
for image in $(find ${dirs[@]} -name '*.png')
do
  ((++i))
  printf "\r[%5d] %-100s " "$i" "$image"
  pngquant 256 < "$image" > "$tempfile"

  oldsize=$(wc -c < "$image")
  newsize=$(wc -c < "$tempfile")
  if [ $newsize -lt $((oldsize * 95 / 100)) ]  # Only accept images that have shrunk by at least 5%
  then
    mv "$tempfile" "$image"
    ((saved += oldsize - newsize))
  else
    ((++skipped))
  fi
done

printf "\n%d images converted, %d images skipped, %d bytes saved.\n" $((i - skipped)) $skipped $saved
