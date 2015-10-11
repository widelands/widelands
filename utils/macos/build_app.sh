#!/bin/bash

set -e

if [ "$#" == "0" ]; then
	echo "Usage: $0 <bzr_repo_directory>"
	exit 1
fi

SOURCE_DIR=$1
REVISION=`bzr revno $SOURCE_DIR`
DESTINATION="WidelandsRelease"
TYPE="Release"
if [[ -f $SOURCE_DIR/WL_RELEASE ]]; then
   WLVERSION="$(cat $SOURCE_DIR/WL_RELEASE)"
else
   WLVERSION="r$REVISION"
fi

echo ""
echo "   Source:      $SOURCE_DIR"
echo "   Version:     $WLVERSION"
echo "   Destination: $DESTINATION"
echo "   Type:        $TYPE"
echo ""

function MakeDMG {
   # Sanity check: Make sure Widelands is there.
   test -f $DESTINATION/Widelands.app/Contents/MacOS/widelands

   find $DESTINATION -name ".?*" -exec rm -v {} \;
   UP=$(dirname $DESTINATION)

   echo "Copying COPYING"
   cp $SOURCE_DIR/COPYING  $DESTINATION/COPYING.txt

   echo "Creating DMG ..."
   hdiutil create -fs HFS+ -volname "Widelands $WLVERSION" -srcfolder "$DESTINATION" "$UP/widelands_64bit_$WLVERSION.dmg"
}

function FixDependencies {
   binary=$1; shift

   echo "Copying dynamic libraries for ${binary} ..."

   dylibbundler -b -of \
      -p '@executable_path/' \
      -d $DESTINATION/Widelands.app/Contents/MacOS \
      -x $DESTINATION/Widelands.app/Contents/MacOS/${binary} 
}

function CopyAndFixDependencies {
   path=$1; shift

   cp $path "$DESTINATION/Widelands.app/Contents/MacOS/"
   chmod 644 "$DESTINATION/Widelands.app/Contents/MacOS/$(basename ${path})"

   FixDependencies "$(basename ${path})"
}

function MakeAppPackage {
   echo "Making $DESTINATION/Widelands.app now."
   rm -Rf $DESTINATION/

   mkdir $DESTINATION/
   mkdir $DESTINATION/Widelands.app/
   mkdir $DESTINATION/Widelands.app/Contents/
   mkdir $DESTINATION/Widelands.app/Contents/Resources/
   mkdir $DESTINATION/Widelands.app/Contents/MacOS/
   cp $SOURCE_DIR/pics/widelands.icns $DESTINATION/Widelands.app/Contents/Resources/widelands.icns
   ln -s /Applications $DESTINATION/Applications

   cat > $DESTINATION/Widelands.app/Contents/Info.plist << EOF
{
   CFBundleName = widelands;
   CFBundleDisplayName = Widelands;
   CFBundleIdentifier = "org.widelands.wl";
   CFBundleVersion = "$WLVERSION";
   CFBundleInfoDictionaryVersion = "6.0";
   CFBundlePackageType = APPL;
   CFBundleSignature = wdld;
   CFBundleExecutable = widelands;
   CFBundleIconFile = "widelands.icns";
}
EOF

   echo "Copying data files ..."
   rsync -Ca $SOURCE_DIR/ $DESTINATION/Widelands.app/Contents/MacOS/ \
      --exclude "build" \
      --exclude "cmake" \
      --exclude "doc" \
      --exclude "locale" \
      --exclude "manual_test" \
      --exclude "po" \
      --exclude "src" \
      --exclude "test" \
      --exclude "utils" \
      --exclude "*.cmake" \
      --exclude "*.py" \
      --exclude "*.sh" \
      --exclude ".*" \
      --exclude "CMakeLists*"

   echo "Copying locales ..."
   rsync -Ca locale $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Copying binary ..."
   cp -a src/widelands $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Stripping binary ..."
   strip -u -r $DESTINATION/Widelands.app/Contents/MacOS/widelands

   FixDependencies widelands

   echo "Copying dynamic libraries for SDL_image ... "
   pushd $DESTINATION/Widelands.app/Contents/MacOS/
   ln -s libpng*.dylib libpng.dylib
   popd
   CopyAndFixDependencies "/usr/local/lib/libjpeg.dylib"  

   echo "Copying dynamic libraries for SDL_mixer ... "
   CopyAndFixDependencies /usr/local/lib/libogg.dylib  
   CopyAndFixDependencies /usr/local/lib/libvorbis.dylib 
   CopyAndFixDependencies /usr/local/lib/libvorbisfile.dylib 
}

function BuildWidelands() {
   cmake $SOURCE_DIR -G Ninja \
      -DCMAKE_CXX_COMPILER:FILEPATH="$(cd $(dirname $0); pwd)/compiler_wrapper.sh" \
      -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING="10.7" \
      -DCMAKE_OSX_SYSROOT:PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk" \
      -DCMAKE_INSTALL_PREFIX:PATH="$DESTINATION/Widelands.app/Contents/MacOS" \
      -DCMAKE_OSX_ARCHITECTURES:STRING="x86_64" \
      -DCMAKE_BUILD_TYPE:STRING="$TYPE" \
      -DCMAKE_PREFIX_PATH:PATH="/usr/local"
   ninja

   echo "Done building."
}

BuildWidelands
MakeAppPackage
MakeDMG
