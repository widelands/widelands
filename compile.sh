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



# TODO  user interaction and functions for installation including a check
# TODO  whether the selected directories are writeable and a password check
# TODO  to become root / Administrator if the dirs are not writeable.



######################################
# Definition of some local variables #
######################################
var_build=0 # 0 == debug(default), 1 == release
var_build_lang=0 # 0 = false 
var_updater=0 # 0 = false
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

  # Ask the user what parts and how Widelands should be build.
  # And save the values
  user_interaction () {
    local_var_ready=0
    while [ $local_var_ready -eq 0 ]
    do
      echo " "
      echo "  Should Widelands be build in [r]elease or [d]ebug mode?"
      echo " "
      read local_var_choice
      echo " "
      case $local_var_choice in
        r) echo "  -> Release mode selected" ; var_build=1 ; local_var_ready=1 ;;
        d) echo "  -> Debug mode selected" ; var_build=0 ; local_var_ready=1 ;;
        *) echo "  -> Bad choice. Please try again!" ;;
      esac
    done
    local_var_ready=0
    if [ $var_build -eq 0 ] ; then
      while [ $local_var_ready -eq 0 ]
      do
        echo " "
        echo "  Should translations be build [y]/[n]?"
        echo " "
        read local_var_choice
        echo " "
        case $local_var_choice in
          y) echo "  -> Translations will be build" ; var_build_lang=1 ; local_var_ready=1 ;;
          n) echo "  -> Translations will not be build" ; var_build_lang=0 ; local_var_ready=1 ;;
          *) echo "  -> Bad choice. Please try again!" ;;
        esac
      done
    fi
    return 0
  }

  # Check if directories / links already exists and create / update them if needed.
  prepare_directories_and_links () {
    # remove build/compile directory (this is the old location)
    if [ -e build/compile ] ; then
      echo " "
      echo "  The build directory has changed"
      echo "  from ./build/compile to ./build."
      echo "  The old directory ./build/compile can be removed."
      echo "  Please backup any files you might not want to lose."
      echo "  Most users can safely say yes here."
      echo "  Do you want to remove the directory ./build/compile? [y]es/[n]o"
      echo " "
      read local_var_choice
      echo " "
      case $local_var_choice in
        y) echo "  -> Removing directory ./build/compile. This may take a while..."
	   rm locale
	   rm -r build/compile || true
	   if [ -e build/compile ] ; then
             echo "  -> Directory could not be removed. This is not fatal, continuing."
	   else
             echo "  -> Directory removed."
	   fi ;;
        n) echo "  -> Left the directory untouched." ;;
        *) echo "  -> Bad choice. Please try again!" ;;
      esac
    fi

    test -d build || mkdir -p build
    test -d build/locale || mkdir -p build/locale
    test -e locale || ln -s build/locale

    cd build

    return 0
  }

  # Compile Widelands
  compile_widelands () {
    var_build_type=""
    if [ $var_build -eq 0 ] ; then
      var_build_type="Debug"
    else
      var_build_type="Release"
    fi

    echo " "
    cmake -DWL_PORTABLE=true .. -DCMAKE_BUILD_TYPE="${var_build_type}"
    make ${MAKEOPTS}
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

  # Ask the user whether an update script should be created and if yes, create it.
  update_script () {
    # First check if this is an bzr checkout at all - only in that case,
    # creation of a script makes any sense.
    if ! [ -f .bzr/branch-format ] ; then
      return 0
    fi
    while :
    do
      echo " "
      echo "  Should I create an update script? [y]es/[n]o"
      echo " "
      read local_var_choice
      echo " "
      case $local_var_choice in
        y) rm -f update.sh || true
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
            echo "touch CMakeLists.txt"
            echo "cd build"
            echo "make ${MAKEOPTS}"
            if [ $var_build_lang -eq 1 ] ; then
              echo "make lang"
            fi
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
           var_updater=1 ; return 0 ;;
        n) echo "  -> No update script has been created." ; return 0 ;;
        *) echo "  -> Bad choice. Please try again!" ;;
      esac
    done
  }
######################################



######################################
#    Here is the "main" function     #
######################################
set -e
basic_check
user_interaction
prepare_directories_and_links
compile_widelands
if [ $var_build_lang -eq 1 ] ; then
  make lang
fi
move_built_files
cd ..
update_script
echo " "
echo "#####################################################"
echo "# Congratulations Widelands was successfully build. #"
echo "# You should now be able to run Widelands via       #"
echo "# typing ./widelands + ENTER in your terminal       #"
if [ $var_updater -eq 1 ] ; then
  echo "#                                                   #"
  echo "# You can update Widelands via running ./update.sh  #"
  echo "# in the same directory you ran this script in.     #"
fi
echo "#####################################################"
######################################
