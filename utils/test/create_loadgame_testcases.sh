#!/bin/sh

# Use after new Widelands releases:
#
# 1. Run this script with the new release to create the savegames
# 2. Commit the created savegames to a new branch in the widelands-test-saves
#    repository
# 3. Create a release tag in the widelands-test-saves repository to generate
#    a downloadable tarball
# 4. Update SAVES_VER in utils/test/download_loadgame_testcases.sh
#    (it should always use saves from the previous stable version, to check
#     compatibility in the new development version)

set -e

# The following environment variables can be set for the script to change
# default values or override autodetection:
# WIDELANDS - path to the widelands binary:
#             must be either absolute, on the PATH, or relative to WL_DIR
# TMP - the temporary directory
# WL_DIR - the main directory of the Widelands source code

if [ -n "$WL_DIR" ] ; then
   WL_DIR=$(dirname $(dirname $(dirname "$0")))
fi

TARGET_DIR=test/save

cd "$WL_DIR"
if ! [ -f regression_test.py -a -d test ] ; then
   echo "ERROR:  Cannot find the main directory of the Widelands source code."
   echo "        Please call 'utils/test/$(basename $0)' from the main directory of"
   echo "        the Widelands source code."
   exit 1
fi
WL_DIR="$(pwd)"

if [ ! -d "$TARGET_DIR" ] ; then
   mkdir "$TARGET_DIR"
fi

if [ -z "$PREFIX" -a -f WL_RELEASE ] ; then
   PREFIX="v$(cat WL_RELEASE)_"
fi

# look for Widelands binary
if [ -z "$WIDELANDS" ] ; then
   if [ -x "${WL_DIR}/widelands" ] ; then
      WIDELANDS="$WL_DIR/widelands"
   elif [ -x "${WL_DIR}/widelands.exe" ] ; then
      WIDELANDS="$WL_DIR/widelands.exe"
   elif hash widelands ; then
      WIDELANDS=widelands
   else
      echo "ERROR: The Widelands binary was not found."
      echo "       You can specify it by setting the WIDELANDS environment variable."
      exit 1
   fi
elif ! hash "$WIDELANDS" ; then
   if hash "${WL_DIR}/${WIDELANDS}" ; then
      WIDELANDS="${WL_DIR}/${WIDELANDS}"
   else
      echo "ERROR: The Widelands binary was not found at '$WIDELANDS'"
      exit 1
   fi
fi

REGTEST_DIR=widelands_regression_test/WidelandsTestCase/save
if [ -n "$TMP" ] ; then
   REGTEST_DIR="${TMP}/${REGTEST_DIR}"
else
   REGTEST_DIR="/tmp/${REGTEST_DIR}"
fi

echo "Creating test savegames of the all_tribes regression tests..."
./regression_test.py -b "$WIDELANDS" -n -k -r all_tribes

if [ ! -d "$REGTEST_DIR" ] ; then
   echo "ERROR: Cannot find save dir of regression tests at '$REGTEST_DIR'"
   exit 1
fi

cd "$REGTEST_DIR"
for F in *.wgf ; do
   mv "$F" "${WL_DIR}/${TARGET_DIR}/${PREFIX}regtest_${F}"
done

echo ""

cd "$WL_DIR"
TMPDIR="$(mktemp -d)"
SAVEFILE="${TMPDIR}/save/test_save.wgf"

echo "Creating test savegames of all campaigns with custom units..."
for S in atl02 emp04 fri02 fri03 fri04 fri05 ; do
   echo "   running ${S}..."
   "$WIDELANDS" --nosound --fail-on-lua-error --language=en_US \
                --messagebox-timeout=1 \
                --homedir="$TMPDIR" \
                --datadir="$WL_DIR"/data \
                --datadir_for_testing="$WL_DIR" \
                --scenario="campaigns/${S}.wmf" --difficulty=1 \
                --script=test/scripting/wait_save_and_quit.lua >"${TMPDIR}/${S}.log"
   if [ ! -f "$SAVEFILE" ] ; then
      echo "ERROR: No savegame found for scenario $S"
      exit 1
   fi
   echo "      done."
   mv "$SAVEFILE" "${TARGET_DIR}/${PREFIX}campaign_${S}.wgf"
done

echo
echo "Done creating savegames for compatibility testing:"
ls -l "$TARGET_DIR"

