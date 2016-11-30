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
   rsync -Ca $SOURCE_DIR/data $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Copying locales ..."
   rsync -Ca locale $DESTINATION/Widelands.app/Contents/MacOS/data/

   echo "Copying binary ..."
   cp -a src/widelands $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Stripping binary ..."
   strip -u -r $DESTINATION/Widelands.app/Contents/MacOS/widelands

   echo "Copying dynamic libraries for SDL_image ... "
   CopyLibrary "$(brew --prefix libpng)/lib/libpng.dylib"
   CopyLibrary "$(brew --prefix jpeg)/lib/libjpeg.dylib"

   echo "Copying dynamic libraries for SDL_mixer ... "
   CopyLibrary $(brew --prefix libogg)/lib/libogg.dylib
   CopyLibrary $(brew --prefix libvorbis)/lib/libvorbis.dylib
   CopyLibrary $(brew --prefix libvorbis)/lib/libvorbisfile.dylib

   $SOURCE_DIR/utils/macos/fix_dependencies.py \
      $DESTINATION/Widelands.app/Contents/MacOS/widelands \
      $DESTINATION/Widelands.app/Contents/MacOS/*.dylib
}

function BuildWidelands() {
   PREFIX_PATH="$(brew --prefix libpng)"
   PREFIX_PATH+=";$(brew --prefix jpeg)"
   PREFIX_PATH+=";$(brew --prefix libpng)"
   PREFIX_PATH+=";$(brew --prefix python)"
   PREFIX_PATH+=";$(brew --prefix zlib)"
   PREFIX_PATH+=";/usr/local"
   PREFIX_PATH+=";/usr/local/Homebrew"

   export SDL2DIR="$(brew --prefix sdl2)"
   export SDL2IMAGEDIR="$(brew --prefix sdl2_image)"
   export SDL2MIXERDIR="$(brew --prefix sdl2_mixer)"
   export SDL2TTFDIR="$(brew --prefix sdl2_ttf)"
   export SDL2NETDIR="$(brew --prefix sdl2_net)"
   export BOOST_ROOT="$(brew --prefix boost)"
   export ICU_ROOT="$(brew --prefix icu4c)"

   cmake $SOURCE_DIR -G Ninja \
      -DCMAKE_C_COMPILER:FILEPATH="$(brew --prefix ccache)/libexec/gcc-6" \
      -DCMAKE_CXX_COMPILER:FILEPATH="$(brew --prefix ccache)/libexec/g++-6" \
      -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING="10.7" \
      -DCMAKE_OSX_SYSROOT:PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk" \
      -DCMAKE_INSTALL_PREFIX:PATH="$DESTINATION/Widelands.app/Contents/MacOS" \
      -DCMAKE_OSX_ARCHITECTURES:STRING="x86_64" \
      -DCMAKE_BUILD_TYPE:STRING="$TYPE" \
      -DGLEW_INCLUDE_DIR:PATH="$(brew --prefix glew)/include" \
      -DGLEW_LIBRARY:PATH="$(brew --prefix glew)/lib/libGLEW.dylib" \
      -DCMAKE_PREFIX_PATH:PATH="${PREFIX_PATH}"
   ninja

   echo "Done building."
}

BuildWidelands
MakeAppPackage
MakeDMG
