#!/bin/sh
echo " "
echo "###########################################################"
echo "#     Script to simplify the compilation of Widelands     #"
echo "###########################################################"
echo " "
echo "  Because of the many different systems Widelands"
echo "  might be compiled on, we unfortunally can not"
echo "  provide a simple way to prepare your system for"
echo "  compilation. To ensure your system is ready, best"
echo "  check http://wl.widelands.org/wiki/BuildingWidelands"
echo " "
echo "  You will often find helpful hands at our"
echo "  * IRC Chat: http://wl.widelands.org/webchat/"
echo "  * Forums: http://wl.widelands.org/forum/"
echo "  * Mailinglist: http://wl.widelands.org/wiki/MailLists/"
echo " "
echo "  Please post your bug reports and feature requests at:"
echo "  https://bugs.launchpad.net/widelands"
echo " "
echo "###########################################################"
echo " "


## Option to avoid building and linking website-related executables.
BUILD_WEBSITE="ON"
BUILD_TRANSLATIONS="ON"
BUILDTYPE="Debug"
while [ "$1" != "" ]; do
  if [ "$1" = "--no-website" -o "$1" = "-w" ]; then
    BUILD_WEBSITE="OFF"
  elif [ "$1" = "--release" -o "$1" = "-r" ]; then
    BUILDTYPE="Release"
  elif [ "$1" = "--no-translations" -o "$1" = "-t" ]; then
    BUILD_TRANSLATIONS="OFF"
  fi
  shift
done
if [ $BUILD_WEBSITE = "ON" ]; then
  echo "A complete build will be created."
  echo "You can use -w or --no-website to omit building and"
  echo "linking website-related executables."
else
  echo "Any website-related code will be OMITTED in the build."
  echo "Make sure that you have created and tested a full"
  echo "build before submitting code to the repository!"
fi
echo " "
if [ $BUILD_TRANSLATIONS = "ON" ]; then
  echo "Translations will be built."
  echo "You can use -t or --no-translations to omit building them."
else
echo "Translations will not be built."
fi
echo " "
echo "###########################################################"
echo " "
if [ $BUILDTYPE = "Release" ]; then
  echo "Creating a Release build."
else
  echo "Creating a Debug build. Use -r to create a Release build."
fi
echo " "
echo "For instructions on how to adjust options and build with"
echo "CMake, please take a look at"
echo "https://wl.widelands.org/wiki/BuildingWidelands/."
echo " "
echo "###########################################################"
echo " "

######################################
# Definition of some local variables #
######################################
buildtool="" #Use ninja by default, fall back to make if that is not available.
######################################


######################################
#    Definition of some functions    #
######################################
  # Check basic stuff
  basic_check () {
    # Check whether the script is run in a widelands main directory
    if ! [ -f src/wlapplication.cc ] ; then
      echo "  This script must be run from the main directory of the widelands"
      echo "  source code."
      exit 1
    fi
    return 0
  }

  set_buildtool () {
    #Defaults to ninja, but if that is not found, we use make instead
    if [ `command -v ninja` ] ; then
      buildtool="ninja"
    #On some systems (most notably Fedora), the binary is called ninja-build
    elif [ `command -v ninja-build` ] ; then
      buildtool="ninja-build"
    #... and some systems refer to GNU make as gmake
    elif [ `command -v gmake` ] ; then
      buildtool="gmake"
    else
      buildtool="make"
    fi
  }

  # Check if directories / links already exists and create / update them if needed.
  prepare_directories_and_links () {
    test -d build/locale || mkdir -p build/locale
    test -e data/locale || ln -s ../build/locale data/locale
    return 0
  }

  # Compile Widelands
  compile_widelands () {
    if [ $buildtool = "ninja" ] || [ $buildtool = "ninja-build" ] ; then
      cmake -G Ninja .. -DCMAKE_BUILD_TYPE=$BUILDTYPE -DOPTION_BUILD_WEBSITE_TOOLS=$BUILD_WEBSITE -DOPTION_BUILD_TRANSLATIONS=$BUILD_TRANSLATIONS
    else
      cmake .. -DCMAKE_BUILD_TYPE=$BUILDTYPE -DOPTION_BUILD_WEBSITE_TOOLS=$BUILD_WEBSITE -DOPTION_BUILD_TRANSLATIONS=$BUILD_TRANSLATIONS
    fi

    $buildtool
    return 0
  }

  # Remove old and move newly compiled files
  move_built_files () {
    rm  -f ../VERSION || true
    rm  -f ../widelands || true

    mv VERSION ../VERSION
    mv src/widelands ../widelands
    return 0
  }

  create_update_script () {
    # First check if this is an bzr checkout at all - only in that case,
    # creation of a script makes any sense.
    if ! [ -f .bzr/branch-format ] ; then
      echo "You don't appear to be using Bazaar. An update script will not be created"
      return 0
    fi
      rm -f update.sh || true
      cat > update.sh << END_SCRIPT
#!/bin/sh
echo "################################################"
echo "#            Widelands update script.          #"
echo "################################################"
echo " "

set -e
if ! [ -f src/wlapplication.cc ] ; then
  echo "  This script must be run from the main directory of the widelands"
  echo "  source code."
  exit 1
fi

bzr pull
cd build
$buildtool
rm  ../VERSION || true
rm  ../widelands || true
mv VERSION ../VERSION
mv src/widelands ../widelands
cd ..

echo " "
echo "################################################"
echo "#      Widelands was updated successfully.     #"
echo "# You should be able to run it via ./widelands #"
echo "################################################"
END_SCRIPT
      chmod +x ./update.sh
      echo "  -> The update script has successfully been created."
  }
######################################



######################################
#    Here is the "main" function     #
######################################
set -e
basic_check
set_buildtool
prepare_directories_and_links
mkdir -p build
cd build
compile_widelands
move_built_files
cd ..
create_update_script
echo " "
echo "###########################################################"
echo "# Congratulations! Widelands has been built successfully  #"
echo "# with the following settings:                            #"
echo "#                                                         #"
if [ $BUILDTYPE = "Release" ]; then
  echo "# - Release build                                         #"
else
  echo "# - Debug build                                           #"
fi
if [ $BUILD_TRANSLATIONS = "ON" ]; then
  echo "# - Translations                                          #"
else
  echo "# - No translations                                       #"
fi

if [ $BUILD_WEBSITE = "ON" ]; then
  echo "# - Website-related executables                           #"
else
  echo "# - No website-related executables                        #"
fi
echo "#                                                         #"
echo "# You should now be able to run Widelands via             #"
echo "# typing ./widelands + ENTER in your terminal             #"
echo "#                                                         #"
echo "# You can update Widelands via running ./update.sh        #"
echo "# in the same directory that you ran this script in.      #"
echo "###########################################################"
######################################
