#!/bin/bash

set -e

# Command line parameters will be passed to compile.sh, check its help for usage

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SOURCE_DIR=$DIR/../../

# Check if the SDK for the minimum build target is available.
# If not, use the one for the installed macOS Version
OSX_MIN_VERSION="12.3"
SDK_DIRECTORY="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$OSX_MIN_VERSION.sdk"

OSX_VERSION=$(sw_vers -productVersion | cut -d . -f 1,2)

if [ ! -d "$SDK_DIRECTORY" ]; then
   OSX_MIN_VERSION=$OSX_VERSION
   SDK_DIRECTORY="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$OSX_VERSION.sdk"
   if [ ! -d "$SDK_DIRECTORY" ]; then
      # If the SDK for the current macOS Version can't be found, use whatever is linked to MacOSX.sdk
      SDK_DIRECTORY="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
   fi
fi

PYTHON=python
if ! which python > /dev/null; then
   if which python3 > /dev/null; then
      PYTHON=python3
   else
      echo "No python executable found!"
   fi
fi
WLVERSION=$("$PYTHON" "$DIR"/../detect_revision.py)

DESTINATION="WidelandsRelease"

if [[ -f "$SOURCE_DIR"/WL_RELEASE ]]; then
   WLVERSION="$(cat "$SOURCE_DIR"/WL_RELEASE)"
fi

WLVERSION="${WLVERSION//\//\~}"  # Fix problems with slashes in branch names

echo ""
echo "   Source:      $SOURCE_DIR"
echo "   Version:     $WLVERSION"
echo "   Destination: $DESTINATION"
echo "   Type:        $TYPE"
echo "   macOS:       $OSX_VERSION"
echo "   Target:      $OSX_MIN_VERSION"
echo "   Compiler:    $COMPILER"
echo ""

function MakeDMG {
   # Sanity check: Make sure Widelands is there.
   test -f $DESTINATION/Widelands.app/Contents/MacOS/widelands

   find $DESTINATION -name ".?*" -exec rm -v {} \;
   UP=$(dirname $DESTINATION)
   DMGFILE="$UP/widelands_${OSX_MIN_VERSION}_${WLVERSION}.dmg"

   echo "Copying COPYING"
   cp "$SOURCE_DIR"/COPYING  "$DESTINATION"/COPYING.txt

   echo "Creating DMG ..."
   SUDO=""
   if [ -n "$GITHUB_ACTION" ]; then
      # Sometimes we get resource busy errors in the github actions
      HDI_MAX_TRIES=3
      # MacOS 13 is the worst
      if [ "${MATRIX_OS}" = 13 ]; then
         echo "Running on MacOS 13, run hdiutil as root"
         SUDO=sudo
      fi
   else
      HDI_MAX_TRIES=1
   fi
   HDI_TRY=0
   while true; do
      HDI_TRY=$(( ++HDI ))
      if $SUDO hdiutil create -verbose -fs APFS -volname "Widelands $WLVERSION" \
                       -srcfolder "$DESTINATION" "$DMGFILE"
      then
         return
      fi
      HDI_RESULT=$?
      if [ $HDI_TRY -eq $HDI_MAX_TRIES ]; then
         exit $HDI_RESULT
      fi
      if [ -n "$GITHUB_ACTION" ]; then
         echo "::warning::hdiutil try $HDI_TRY error code: ${HDI_RESULT}... retrying"
      fi
      if [ -f "$DMGFILE" ]; then
        rm "$DMGFILE"
      fi
      # XProtect is one of the possible causes of the resource busy errors
      sudo pkill -9 XProtect
      echo "  will retry after 20 seconds..."
      sleep 20
   done
}

function MakeAppPackage {
   echo "Making $DESTINATION/Widelands.app now."
   rm -Rf "$DESTINATION"

   mkdir "$DESTINATION"
   mkdir "$DESTINATION"/Widelands.app
   mkdir "$DESTINATION"/Widelands.app/Contents
   mkdir "$DESTINATION"/Widelands.app/Contents/Resources
   mkdir "$DESTINATION"/Widelands.app/Contents/MacOS
   cp "$SOURCE_DIR"/data/images/logos/widelands.icns "$DESTINATION"/Widelands.app/Contents/Resources/widelands.icns
   ln -s /Applications "$DESTINATION"/Applications

   cat > $DESTINATION/Widelands.app/Contents/Info.plist << EOF
{
   CFBundleName = widelands;
   CFBundleDisplayName = Widelands;
   CFBundleIdentifier = "org.widelands.wl";
   CFBundleVersion = "$WLVERSION";
   CFBundleShortVersionString = "$WLVERSION";
   CFBundleInfoDictionaryVersion = "6.0";
   CFBundlePackageType = APPL;
   CFBundleSignature = wdld;
   CFBundleExecutable = widelands;
   CFBundleIconFile = "widelands.icns";
}
EOF

   echo "Copying data files ..."
   rsync -Ca "$SOURCE_DIR"/data "$DESTINATION"/Widelands.app/Contents/MacOS/

   echo "Copying binary ..."
   cp -a "$SOURCE_DIR"/widelands $DESTINATION/Widelands.app/Contents/MacOS/

   # Locate ASAN Library by asking llvm (nice trick by SirVer I suppose)
   ASANLIB=$(echo "int main(void){return 0;}" | xcrun clang -fsanitize=address \
       -xc -o/dev/null -v - 2>&1 |   tr ' ' '\n' | grep libclang_rt.asan_osx_dynamic.dylib)

   ASANPATH=$(dirname "$ASANLIB")

   echo "Copying and fixing dynamic libraries... "
   # $SOURCE_DIR/utils/macos/bundle-dylibs.sh \
   #  -l ../libs \
   #  $DESTINATION/Widelands.app

   # Alternative Tool to use
   dylibbundler --overwrite-dir --bundle-deps --no-codesign \
	--search-path "$ASANPATH" \
	--fix-file "$DESTINATION/Widelands.app/Contents/MacOS/widelands" \
	--dest-dir "$DESTINATION/Widelands.app/Contents/libs"

   echo "Re-sign libraries with 'ad-hoc signing' see man codesign"
   codesign --sign - --force $DESTINATION/Widelands.app/Contents/libs/*

   echo "Stripping binary ..."
   strip -u -r $DESTINATION/Widelands.app/Contents/MacOS/widelands
}

function BuildWidelands() {
   PREFIX_PATH=$(brew --prefix)
   eval "$("$PREFIX_PATH/bin/brew shellenv")"
   export CMAKE_PREFIX_PATH="${PREFIX_PATH}/opt/icu4c"

   echo "FIXED ICU Issue $CMAKE_PREFIX_PATH"

   pushd "$SOURCE_DIR"
   ./compile.sh \
      -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING="$OSX_MIN_VERSION" \
      -DCMAKE_OSX_SYSROOT:PATH="$SDK_DIRECTORY" \
      "$@"
   popd

   echo "Done building."
}

BuildWidelands "$@"
MakeAppPackage
MakeDMG
