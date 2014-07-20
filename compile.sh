#!/bin/sh
echo " "
echo "################################################"
echo "# Script to simplify compilations of Widelands #"
echo "################################################"
echo " "
echo "  Because of the many different systems Widelands"
echo "  might be compiled on, we unfortunally can not"
echo "  provide a simple way to prepare your system for"
echo "  compilation. To ensure your system is ready, best"
echo "  check http://wl.widelands.org/wiki/BuildingWidelands"
echo " "
echo "  You often find helpfully hands at our"
echo "  * IRC Chat: http://wl.widelands.org/webchat/"
echo "  * Forums: http://wl.widelands.org/forum/"
echo "  * Mailinglist: http://wl.widelands.org/wiki/MailLists/"
echo " "
echo "  Please post your bugreports and feature requests at:"
echo "  https://bugs.launchpad.net/widelands"
echo " "
echo "################################################"
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
    #TODO(code review): Are these returns in various methods necessary?
    #It doesn't seem like anything ever checks the return, and in case
    #of something bad, it just calls exit anyways (see above)
    return 0
  }

  set_buildtool () {
    #Defaults to ninja, but if that is not found, we use make instead
    if [ `command -v ninja` ] ; then
      buildtool="ninja"
    #On some systems (most notably Fedora), the binary is called ninja-build
    elif [ `command -v ninja-build` ] ; then
      buildtool="ninja-build"
    else
      buildtool="make"
    fi
  }

  # Check if directories / links already exists and create / update them if needed.
  prepare_directories_and_links () {
    test -d build/locale || mkdir -p build/locale
    test -e locale || ln -s build/locale

    return 0
  }

  # Compile Widelands
  compile_widelands () {
    echo "Builds a debug build by default. If you want a Release build, "
    echo "you will need to build it manually passing the"
    echo "option -DCMAKE_BUILD_TYPE=\"Release\" to cmake"

    #TODO(code review): WL_PORTABLE might be going away, see https://bugs.launchpad.net/widelands/+bug/1342228

    if [ $buildtool = "ninja" ] || [ $buildtool = "ninja-build" ] ; then
      cmake -G Ninja -DWL_PORTABLE=true .. -DCMAKE_BUILD_TYPE="Debug"
    else
      cmake -DWL_PORTABLE=true .. -DCMAKE_BUILD_TYPE="Debug"
    fi

    #TODO(code review): Do we need to pass in makeopts, is it likely that people running this script will
    #have makeopts set? Also, I assume ninja will be able to deal with this, if it is a drop-in replacement
    $buildtool ${MAKEOPTS}
    #TODO(code review): Ideally lang is always run as just another part of the line above
    $buildtool lang
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
           (echo "#!/bin/sh"
            echo "echo \" \""
            echo "echo \"################################################\""
            echo "echo \"#            Widelands update script.          #\""
            echo "echo \"################################################\""
            echo "echo \" \""
            echo " "
            echo "set -e"
            echo "if ! [ -f src/wlapplication.cc ] ; then"
            echo "  echo \"  This script must be run from the main directory of the widelands\""
            echo "  echo \"  source code.\""
            echo "  exit 1"
            echo "fi"
            echo " "
            echo "bzr pull"
            echo "cd build"
            echo "cmake .."
            echo "$buildtool ${MAKEOPTS}"
            echo "$buildtool lang"
            echo "rm  ../VERSION || true"
            echo "rm  ../widelands || true"
            echo "mv VERSION ../VERSION"
            echo "mv src/widelands ../widelands"
            echo "cd .."
            echo " "
            echo "echo \" \""
            echo "echo \"################################################\""
            echo "echo \"#      Widelands was updated successfully.     #\""
            echo "echo \"# You should be able to run it via ./widelands #\""
            echo "echo \"################################################\""
           ) > update.sh
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
cd build
compile_widelands
move_built_files
cd ..
create_update_script
echo " "
echo "#####################################################"
echo "# Congratulations Widelands was successfully build. #"
echo "# You should now be able to run Widelands via       #"
echo "# typing ./widelands + ENTER in your terminal       #"
echo "#                                                   #"
echo "# You can update Widelands via running ./update.sh  #"
echo "# in the same directory you ran this script in.     #"
echo "#####################################################"
######################################
