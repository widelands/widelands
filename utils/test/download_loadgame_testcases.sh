#!/bin/sh

## Download saveloading compatibility test cases for regression_test.py

SAVES_VER=1.2

URL="https://github.com/tothxa/widelands-test-saves/archive/refs/tags/v${SAVES_VER}.tar.gz"

TMPDIR="$(mktemp -d)"
DL_TARGET="${TMPDIR}/widelands-test-saves-${SAVES_VER}.tar.gz"
EXTRACTED_DIR="${TMPDIR}/widelands-test-saves-${SAVES_VER}/test/save"

echo "Downloading v${SAVES_VER} saveloading test cases..."

WL_DIR=$(dirname $(dirname $(dirname "$0")))
cd "$WL_DIR"
WL_DIR="$(pwd)"
if ! [ -f src/wlapplication.cc -a -d test ] ; then
   echo
   echo "ERROR:  Cannot find the main directory of the Widelands source code."
   echo "        Please call 'utils/test/$(basename $0)' from the main directory of"
   echo "        Widelands source where you would like to install the test cases."
   echo
   exit 1
fi

if ! [ -d test/save ] ; then
   mkdir test/save
fi

if hash wget 2>/dev/null ; then
   DOWNLOADER="wget -O"
elif hash curl 2>/dev/null ; then
   DOWNLOADER="curl -Lo"
else
   echo "Cannot find a suitable download tool."
   echo
   exit 1
fi

$DOWNLOADER "$DL_TARGET" "$URL"

cd "$TMPDIR"
tar -xzf "$DL_TARGET"
cd "$WL_DIR"

cp -R -P -p "$EXTRACTED_DIR"/* "$WL_DIR"/test/save
ls -l -R "$WL_DIR"/test/save
