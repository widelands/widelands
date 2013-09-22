#!/bin/bash

set -e 

SOURCE_DIR="../../wl_bzr/"
REVISION=`bzr revno $SOURCE_DIR`
DESTINATION="WidelandsRelease"
TYPE="Release"
echo ""
echo "   Source:      $SOURCE_DIR"
echo "   Revision:    bzr$REVISION"
echo "   Destination: $DESTINATION"
echo "   Type:        $TYPE"
echo ""


function MakeAppPackage {
   rm -Rf $DESTINATION/

   mkdir $DESTINATION/
   mkdir $DESTINATION/Widelands.app/
   mkdir $DESTINATION/Widelands.app/Contents/
   mkdir $DESTINATION/Widelands.app/Contents/Resources/
   mkdir $DESTINATION/Widelands.app/Contents/MacOS/
   cp $SOURCE_DIR/pics/widelands.icns $DESTINATION/Widelands.app/Contents/Resources/widelands.icns
   ln -s /Applications $DESTINATION/Applications

   echo "{
      CFBundleName = widelands;
      CFBundleDisplayName = Widelands;
      CFBundleIdentifier = "org.widelands.wl";
      CFBundleVersion = bzr$REVISION;
      "CFBundleInfoDictionaryVersion" = "6.0";
      CFBundlePackageType = APPL;
      CFBundleSignature = wdld;
      CFBundleExecutable = widelands;
      CFBundleIconFile = "widelands.icns";
   }" > $DESTINATION/Widelands.app/Contents/Info.plist 

   echo "Copying data files ..."
   rsync -Ca $SOURCE_DIR/ $DESTINATION/Widelands.app/Contents/MacOS/ \
      --exclude "doc" \
      --exclude "locale" \
      --exclude "src" \
      --exclude "cmake" \
      --exclude "CMakeLists*" \
      --exclude "*.cmake" \
      --exclude "build" \
      --exclude "po" \
      --exclude "test" \
      --exclude "manual_test" \
      --exclude "utils" \
      --exclude "*.py" \
      --exclude "*.sh"

   echo "Copying locales ..."
   rsync -Ca locale $DESTINATION/Widelands.app/Contents/MacOS/

   echo "Stripping and copying binary ..."
   cp -a src/widelands $DESTINATION/Widelands.app/Contents/MacOS/
   strip -u -r $DESTINATION/Widelands.app/Contents/MacOS/widelands 
	
   echo "Copying dynamic libraries over ...."
   dylibbundler -od -b -x $DESTINATION/Widelands.app/Contents/MacOS/widelands  -d $DESTINATION/Widelands.app/Contents/libs

   if [[ ! -f  $DESTINATION/Widelands.app/Contents/MacOS/VERSION ]]; then
      echo "### WARNING: No VERSION file found. Hopefully this is a nightly."
   fi
}

function BuildWidelands() {
   cmake $SOURCE_DIR $GENERATOR \
      -DCMAKE_CXX_COMPILER:FILEPATH="$(pwd)/compiler_wrapper.sh" \
      -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING="10.7" \
      -DCMAKE_OSX_SYSROOT:PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk" \
      -DCMAKE_INSTALL_PREFIX:PATH="$DESTINATION/Widelands.app/Contents/MacOS" \
      -DCMAKE_OSX_ARCHITECTURES:STRING="x86_64" \
      -DCMAKE_BUILD_TYPE:STRING="$TYPE" \
      -DCMAKE_PREFIX_PATH:PATH="/usr/local" \
      \
      -DSDL_LIBRARY:STRING="-L/usr/local/lib /usr/local/lib/libSDLmain.a /usr/local/lib/libSDL.a -Wl,-framework,OpenGL -Wl,-framework,Cocoa -Wl,-framework,ApplicationServices -Wl,-framework,Carbon -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,IOKit" \
      -DSDL_INCLUDE_DIR:PATH="/usr/local/include/SDL" \
      \
      -DSDLIMAGE_LIBRARY:STRING="-Wl,/usr/local/lib/libSDL_image.a -Wl,/usr/local/lib/libjpeg.a" \
      -DSDLIMAGE_INCLUDE_DIR:PATH="/usr/local/include/SDL" \
      \
      -DPNG_LIBRARY:FILEPATH="/usr/local/opt/libpng/lib/libpng.a" \
      -DPNG_INCLUDE_DIR:PATH="/usr/local/opt/libpng/include" \
      \
      -DSDLTTF_LIBRARY:STRING="-Wl,/usr/local/opt/freetype/lib/libfreetype.a -Wl,/usr/local/lib/libbz2.a -Wl,/usr/local/lib/libSDL_ttf.a" \
      -DSDLTTF_INCLUDE_DIR:PATH="/usr/local/include/SDL" \
      \
      -DSDLGFX_LIBRARY:FILEPATH="/usr/local/lib/libSDL_gfx.a" \
      -DSDLGFX_INCLUDE_DIR:PATH="/usr/local/include/SDL" \
      \
      -DSDLMIXER_LIBRARY:STRING="-Wl,/usr/local/lib/libvorbisfile.a -Wl,/usr/local/lib/libogg.a -Wl,/usr/local/lib/libvorbis.a -Wl,/usr/local/lib/libSDL_mixer.a" \
      -DSDLMIXER_INCLUDE_DIR:PATH="/usr/local/include/SDL" \
      \
      -DSDLNET_LIBRARY:FILEPATH="/usr/local/lib/libSDL_net.a" \
      -DSDLNET_INCLUDE_DIR:PATH="/usr/local/include/SDL" \
      \
      -DINTL_LIBRARY:STRING="-Wl,/usr/local/opt/libiconv/lib/libiconv.a -Wl,/usr/local/opt/gettext/lib/libintl.a" \
      -DINTL_INCLUDE_DIR:PATH="/usr/local/opt/gettext/include" \
      \
      -DGLEW_LIBRARY:FILEPATH="/usr/local/lib/libGLEW.a" \
      -DGLEW_INCLUDE_DIR:PATH="/usr/local/include/gl" \
      \
      -DZLIB_LIBRARY:FILEPATH="/usr/local/opt/zlib/lib/libz.a" \
      -DZLIB_INCLUDE_DIR:PATH="/usr/local/include" \
      \
      -DLUA_LIBRARIES:STRING="/usr/local/lib/liblua.a" \
      -DLUA_INCLUDE_DIR:PATH="/usr/local/include" \
      -DLUA_LIBRARY:FILEPATH="/usr/local/lib/liblua.a" \
      \
      -DBoost_USE_STATIC_LIBS="ON"

   make -j2

   echo "Done building. Check otool output for non System libraries."
   otool -L src/widelands | egrep -v '\s\/System'
}

BuildWidelands
MakeAppPackage

