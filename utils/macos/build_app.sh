#!/bin/bash

set -e

USAGE="See compile.sh"

SOURCE_DIR=../

# Check if the SDK for the minimum build target is available.
# If not, use the one for the installed macOS Version
OSX_MIN_VERSION="10.7"
SDK_DIRECTORY="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$OSX_MIN_VERSION.sdk"

OSX_VERSION=$(sw_vers -productVersion | cut -d . -f 1,2)
OSX_MINOR=$(sw_vers -productVersion | cut -d . -f 2)

if [ ! -d "$SDK_DIRECTORY" ]; then
   if [ "$OSX_MINOR" -ge 9 ]; then
      OSX_MIN_VERSION="10.9"
   fi
   SDK_DIRECTORY="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX$OSX_VERSION.sdk"
   if [ ! -d "$SDK_DIRECTORY" ]; then
      # If the SDK for the current macOS Version can't be found, use whatever is linked to MacOSX.sdk
      SDK_DIRECTORY="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
   fi
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REVISION=`python $DIR/../detect_revision.py`

DESTINATION="WidelandsRelease"

if [[ -f $SOURCE_DIR/WL_RELEASE ]]; then
   WLVERSION="$(cat $SOURCE_DIR/WL_RELEASE)"
else
   WLVERSION="$REVISION"
fi

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

   echo "Copying COPYING"
   cp $SOURCE_DIR/COPYING  $DESTINATION/COPYING.txt

   echo "Creating DMG ..."
   # if [ "$TYPE" == "Release" ]; then
      hdiutil create -fs HFS+ -volname "Widelands $WLVERSION" -srcfolder "$DESTINATION" "$UP/widelands_${OSX_MIN_VERSION}_${WLVERSION}.dmg"
   # elif [ "$TYPE" == "Debug" ]; then
   #  hdiutil create -fs HFS+ -volname "Widelands $WLVERSION" -srcfolder "$DESTINATION" "$UP/widelands_${OSX_MIN_VERSION}_${WLVERSION}_${TYPE}.dmg"
   # fi
}

function CopyLibrary {
   path=$1; shift
   cp $path "$DESTINATION/Widelands.app/Contents/MacOS/"
   chmod 644 "$DESTINATION/Widelands.app/Contents/MacOS/$(basename ${path})"
}

function MakeAppPackage {
   echo "Making $DESTINATION/Widelands.app now."
   rm -Rf $DESTINATION/

   mkdir $DESTINATION/
   mkdir $DESTINATION/Widelands.app/
   mkdir $DESTINATION/Widelands.app/Contents/
   mkdir $DESTINATION/Widelands.app/Contents/Resources/
   mkdir $DESTINATION/Widelands.app/Contents/MacOS/
   cp $SOURCE_DIR/data/images/logos/widelands.icns $DESTINATION/Widelands.app/Contents/Resources/widelands.icns
   ln -s /Applications $DESTINATION/Applications

   # TODO(stonerl/k.halfmann): Check if NSHighResolutionCapable = false; is still neede with #3542
   # is resolved. This needs an updated SDL2.
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
   rsync -Ca $SOURCE_DIR/data $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Copying locales ..."
   rsync -Ca $SOURCE_DIR/data/locale $DESTINATION/Widelands.app/Contents/MacOS/data/

   echo "Copying binary ..."
   cp -a $SOURCE_DIR/widelands $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Copying and fixing dynamic libraries... "
   dylibbundler --create-dir --bundle-deps \
	--fix-file $DESTINATION/Widelands.app/Contents/MacOS/widelands \
	--dest-dir $DESTINATION/Widelands.app/Contents/libs

   echo "Stripping binary ..."
   strip -u -r $DESTINATION/Widelands.app/Contents/MacOS/widelands
}

function BuildWidelands() {
   PREFIX_PATH=$(brew --prefix)
   eval "$($PREFIX_PATH/bin/brew shellenv)"
   export CMAKE_PREFIX_PATH="${PREFIX_PATH}/opt/icu4c"

   echo "FIXED ICU Issue $CMAKE_PREFIX_PATH"

   pushd ../
   ./compile.sh $@
   popd

   echo "Done building."
}

BuildWidelands $@
MakeAppPackage
MakeDMG
