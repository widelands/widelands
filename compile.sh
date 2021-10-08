#!/bin/sh

print_help () {
    # Print help for our options
    echo "###########################################################"
    echo " "
    echo "  Because of the many different systems Widelands"
    echo "  might be compiled on, we unfortunally can not"
    echo "  provide a simple way to prepare your system for"
    echo "  compilation. To ensure your system is ready, best"
    echo "  check https://www.widelands.org/wiki/BuildingWidelands"
    echo " "
    echo "  You will often find helpful hands at our"
    echo "  * IRC Chat: https://www.widelands.org/webchat/"
    echo "  * Forums: https://www.widelands.org/forum/"
    echo "  * Mailinglist: https://www.widelands.org/wiki/MailLists/"
    echo " "
    echo "  Please post your bug reports and feature requests at:"
    echo "  https://github.com/widelands/widelands/issues"
    echo " "
    echo "###########################################################"
    echo " "
    echo "Per default, this script will create a full debug build."
    echo "Unless explicitly switched off, AddressSanitizer will"
    echo "be used as well with debug builds."
    echo " "
    echo "You can override the defaults locally by creating a file"
    echo "called '.compile_defaults' listing your desired command"
    echo "line options in a single line. Only the first line of the"
    echo "file is read, further lines can be used as comments."
    echo "The 'COMPILE_DEFAULTS' environment variable can be set to"
    echo "the name of a different file to use, or if set to a"
    echo "non-existent name, to use the built-in defaults."
    echo " "
    echo "The following options are available:"
    echo " "
    echo "-h or --help          Print this help."
    echo "-q or --quiet         Suppress most of compile.sh's output."
    echo "-v or --verbose       Make compile.sh's output verbose."
    echo " "
    echo "Omission options and their overrides:"
    echo " "
    echo "-w or --no-website    Omit building of website binaries."
    echo "+w or --with-website  Enable building of website binaries."
    echo " "
    echo "-t or --no-translations"
    echo "                      Omit building translations."
    echo "+t or --with-translations"
    echo "                      Enable building translations."
    echo " "
    echo "-s or --skip-tests    Skip linking and executing the tests."
    echo "+s or --do-tests      Link and execute the tests."
    echo " "
    echo "-a or --no-asan       If in debug mode, switch off the AddressSanitizer."
    echo "                      Release builds are created without AddressSanitizer"
    echo "                      by default."
    echo "+a or --with-asan     If in release mode, switch on the AddressSanitizer."
    echo "                      Debug builds are created with AddressSanitizer by"
    echo "                      default."
    echo " "
    echo "-x or --without-xdg   Disable support for the XDG Base Directory Specification."
    echo "+x or --with-xdg      Enable support for the XDG Base Directory Specification."
    echo " "
    echo "Compiler options:"
    echo " "
    echo "-j <number> or --cores <number>"
    echo "                      Set the number of processor cores to use for"
    echo "                      compiling and linking. Default is to leave 1 core"
    echo "                      free."
    echo " "
    echo "-r or --release       Create a release build. If this is not set,"
    echo "                      a debug build will be created."
    echo "-d or --debug         Create a debug build. This is the default,"
    echo "                      unless overridden locally."
    echo " "
    if which g++ >/dev/null; then # gcc specific
    echo "-c or --no-cross-opt  Do not use cross compile unit optimization,"
    echo "                      even if available."
    echo "+c or --with-cross-opt"
    echo "                      Use cross compile unit optimization if available."
    echo " "
    fi
    echo "--gcc                 Try to build with GCC rather than the system default."
    echo "                      If you built with Clang before, you will have to clean"
    echo "                      your build directory before switching compilers."
    echo "                      Expects that the compiler is in '/usr/bin/'."
    echo " "
    echo "--clang               Try to build with Clang rather than the system default."
    echo "                      If you built with GCC before, you will have to clean"
    echo "                      your build directory before switching compilers."
    echo "                      Expects that the compiler is in '/usr/bin/'."
    echo " "
    echo "--default-compiler    Use the system default compiler."
    echo "                      If you built with a different compiler before, you will"
    echo "                      have to clean your build directory before switching"
    echo "                      compilers."
    echo " "
    echo "For the AddressSanitizer output to be useful, some systems (e.g. Ubuntu Linux)"
    echo "require that you set a symlink to the symbolizer. For example:"
    echo " "
    echo "    sudo ln -s /usr/bin/llvm-symbolizer-3.8 /usr/bin/llvm-symbolizer"
    echo " "
    echo "More info about AddressSanitizer at:"
    echo " "
    echo "    https://clang.llvm.org/docs/AddressSanitizer.html"
    echo " "
    echo " "
    echo "CMake options:"
    echo " "
    echo "-D...                 Any option starting with -D will be passed to cmake"
    echo "                      unchanged before the above options."
    echo " "
    return
  }

## Options to control the build.
BUILD_WEBSITE="ON"
BUILD_TRANSLATIONS="ON"
BUILD_TESTS="ON"
BUILD_TYPE="Debug"
USE_FLTO="yes"
USE_ASAN="default"
USE_ASAN_DEFAULT="ON"
COMPILER="default"
USE_XDG="ON"
EXTRA_OPTS=""
# Option for this script itself
QUIET=0

if [ -z "$COMPILE_DEFAULTS" ]; then
  COMPILE_DEFAULTS=.compile_defaults
fi

# try to set default number of cores automatically

# nproc is a GNU tool, may not be available everywhere, but it is preferable,
# when it is available
if hash nproc 2>/dev/null ; then
  MAXCORES="$(nproc)"
else
  # try BSD / OS X
  if hash sysctl 2>/dev/null ; then
    MAXCORES="$(sysctl -n hw.ncpu)" 2>/dev/null
    # we will check result later
  fi
fi

# this has the marginally useful side-effect that MAXCORES can be set in the
# environment when neither method works...
if [ $MAXCORES -gt 0 ] 2>/dev/null ; then
  if [ $MAXCORES -gt 1 ]; then
    CORES=$((MAXCORES - 1))
  else
    CORES=1
  fi
else
  echo "Cannot determine number of available CPU cores. Only one core will be"
  echo "used by default. User provided -j will be trusted."
  echo " "
  MAXCORES=0
  CORES=1
fi

OLD_CLI_ARGS="$@"
if [ -f "$COMPILE_DEFAULTS" -a -r "$COMPILE_DEFAULTS" ]; then
  read LOCAL_DEFAULTS <"$COMPILE_DEFAULTS"
  # We want $LOCAL_DEFAULTS to be split, so no "" for it
  set -- $LOCAL_DEFAULTS "$@"
fi

while [ $# -gt 0 ]
do
  case $1 in
    -a|--no-asan)
      USE_ASAN="OFF"
    shift
    ;;
    +a|--with-asan)
      USE_ASAN="ON"
    shift
    ;;
    -h|--help)
      print_help
      exit 0
    shift
    ;;
    -j|--cores)
      case $2 in
        *[^0-9]*|'')
          echo "Call -j/--cores with a number, e.g. '-j $MAXCORES'"
          exit 1
        ;;
      esac
      if [ "$2" -lt 1 ]; then
        echo "Invalid number of cores was specified: $2"
        exit 1
      fi
      if [ "$MAXCORES" -ge "$2" -o "$MAXCORES" -eq 0 ]; then
        CORES="$2"
      else
        echo "Cannot set number of cores to $2, because the maximum number"
        echo "of supported cores is $MAXCORES."
        exit 1
      fi
    shift 2 # past argument and value
    ;;
    -r|--release)
      BUILD_TYPE="Release"
      if [ "${USE_ASAN}" = "default" ]; then
        USE_ASAN="OFF"
      fi
    shift
    ;;
    -d|--debug)
      BUILD_TYPE="Debug"
      if [ "${USE_ASAN}" = "default" ]; then
        USE_ASAN="ON"
      fi
    shift
    ;;
    -t|--no-translations)
      BUILD_TRANSLATIONS="OFF"
    shift
    ;;
    +t|--with-translations)
      BUILD_TRANSLATIONS="ON"
    shift
    ;;
    -c|--no-cross-opt)
      USE_FLTO="no"
    shift
    ;;
    +c|--with-cross-opt)
      USE_FLTO="yes"
    shift
    ;;
    -s|--skip-tests)
      BUILD_TESTS="OFF"
    shift
    ;;
    +s|--do-tests)
      BUILD_TESTS="ON"
    shift
    ;;
    -w|--no-website)
      BUILD_WEBSITE="OFF"
    shift
    ;;
    +w|--with-website)
      BUILD_WEBSITE="ON"
    shift
    ;;
    -q|--quiet)
      QUIET=1
    shift
    ;;
    -v|--verbose)
      QUIET=0
    shift
    ;;
    --gcc)
      if [ -f /usr/bin/gcc -a /usr/bin/g++ ]; then
        COMPILER=gcc
        export CC=/usr/bin/gcc
        export CXX=/usr/bin/g++
      fi
    shift
    ;;
    --clang)
      if [ -f /usr/bin/clang -a /usr/bin/clang++ ]; then
        COMPILER=clang
        export CC=/usr/bin/clang
        export CXX=/usr/bin/clang++
      fi
    shift
    ;;
    --default-compiler)
      COMPILER=default
      unset CC
      unset CXX
    shift
    ;;
    -x|--without-xdg)
      USE_XDG="OFF"
    shift
    ;;
    +x|--with-xdg)
      USE_XDG="ON"
    shift
    ;;
    -D*)
      EXTRA_OPTS="$EXTRA_OPTS $1"
    shift
    ;;
    *)
      # unknown option
      echo "Unknown option:" $1
      echo "Use -h or --help for list of available options"
      exit 1
    ;;
  esac
done

if [ "${USE_ASAN}" = "default" ]; then
  USE_ASAN="${USE_ASAN_DEFAULT}"
fi

if [ $QUIET -eq 0 ]; then
  echo " "
  echo "###########################################################"
  echo "#     Script to simplify the compilation of Widelands     #"
  echo "###########################################################"
  echo " "

  if [ -n "$LOCAL_DEFAULTS" ]; then
    echo "Using default compile options from '$COMPILE_DEFAULTS':"
    echo "   $LOCAL_DEFAULTS"
    echo " "
    echo "Command line options:"
    echo "   $OLD_CLI_ARGS"
    echo " "
  fi
fi

## Get command and options to use in update.sh
COMMANDLINE="$0"
CMD_ADD () {
  COMMANDLINE="$COMMANDLINE $@"
}

if [ $QUIET -ne 0 ]; then

  CMD_ADD "$LOCAL_DEFAULTS $OLD_CLI_ARGS"

else  # Start of verbose output section

if [ -n "$EXTRA_OPTS" ]; then
  echo "Extra CMake options used: $EXTRA_OPTS"
  echo " "
  CMD_ADD "$EXTRA_OPTS"
fi

echo "Using ${CORES} core(s)."
echo ""
CMD_ADD "--cores ${CORES}"

echo -n "Using compiler: "
if [ $COMPILER = default ]; then
  echo "system default"
  CMD_ADD "--default-compiler"
else
  echo "$COMPILER"
  CMD_ADD "--$COMPILER"
fi
echo " "

if [ $USE_FLTO = "yes" ]; then
  echo "Using cross compile unit optimization if available."
  CMD_ADD "--with-cross-opt"
else
  echo "Not using cross compile unit optimization."
  CMD_ADD "--no-cross-opt"
fi
echo " "

if [ $BUILD_WEBSITE = "ON" ]; then
  echo "A complete build will be created."
  echo "You can use -w or --no-website to omit building and"
  echo "linking website-related executables."
  CMD_ADD "--with-website"
else
  echo "Any website-related code will be OMITTED in the build."
  echo "Make sure that you have created and tested a full"
  echo "build before submitting code to the repository!"
  echo "You can use +w or --with-website to build and link"
  echo "website-related executables."
  CMD_ADD "--no-website"
fi
echo " "
if [ $BUILD_TRANSLATIONS = "ON" ]; then
  echo "Translations will be built."
  echo "You can use -t or --no-translations to omit building them."
  CMD_ADD "--with-translations"
else
  echo "Translations will not be built."
  echo "You can use +t or --with-translations to build them."
  CMD_ADD "--no-translations"
fi
echo " "
if [ $BUILD_TESTS = "ON" ]; then
  echo "Tests will be built."
  echo "You can use -s or --skip-tests to omit building them."
  CMD_ADD "--do-tests"
else
  echo "Tests will not be built."
  echo "You can use +s or --do-tests to build them."
  CMD_ADD "--skip-tests"
fi
echo " "
echo "###########################################################"
echo " "
if [ $BUILD_TYPE = "Release" ]; then
  echo "Creating a Release build. Use -d to create a Debug build."
  CMD_ADD "--release"
else
  echo "Creating a Debug build. Use -r to create a Release build."
  CMD_ADD "--debug"
fi
echo " "
if [ $USE_ASAN = "ON" ]; then
  echo "Will build with AddressSanitizer."
  echo "https://clang.llvm.org/docs/AddressSanitizer.html"
  echo "You can use -a or --no-asan to switch it off."
  CMD_ADD "--with-asan"
else
  echo "Will build without AddressSanitizer."
  echo "You can use +a or --with-asan to switch it on."
  CMD_ADD "--no-asan"
fi
if [ $USE_XDG = "ON" ]; then
  echo " "
  echo "Basic XDG Base Directory Specification will be used on Linux"
  echo "if no existing \$HOME/.widelands folder is found."
  echo "The widelands user data can be found in \$XDG_DATA_HOME/widelands"
  echo "and defaults to \$HOME/.local/share/widelands."
  echo "The widelands user configuration can be found in \$XDG_CONFIG_HOME/widelands"
  echo "and defaults to \$HOME/.config/widelands."
  echo "See https://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html"
  echo "for more information."
  echo " "
  CMD_ADD "--with-xdg"
else
  CMD_ADD "--without-xdg"
fi
echo " "
echo "###########################################################"
echo " "
echo "Call 'compile.sh -h' or 'compile.sh --help' for help."
echo ""
echo "For instructions on how to adjust options and build with"
echo "CMake, please take a look at"
echo "https://www.widelands.org/wiki/BuildingWidelands/."
echo " "
echo "###########################################################"
echo " "

fi  # End of verbose output section

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
    GENERATOR=""
    #Defaults to ninja, but if that is not found, we use make instead
    if [ `command -v ninja` ] ; then
      buildtool="ninja"
      GENERATOR="-G Ninja"
    #On some systems (most notably Fedora), the binary is called ninja-build
    elif [ `command -v ninja-build` ] ; then
      buildtool="ninja-build"
      GENERATOR="-G Ninja"
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
    cmake $GENERATOR .. $EXTRA_OPTS -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DOPTION_BUILD_WEBSITE_TOOLS=$BUILD_WEBSITE -DOPTION_BUILD_TRANSLATIONS=$BUILD_TRANSLATIONS -DOPTION_BUILD_TESTS=$BUILD_TESTS -DOPTION_ASAN=$USE_ASAN -DUSE_XDG=$USE_XDG -DUSE_FLTO_IF_AVAILABLE=${USE_FLTO}

    $buildtool -j $CORES

    return 0
  }

  # Remove old and move newly compiled files
  move_built_files () {
    rm  -f ../VERSION || true
    rm  -f ../widelands || true

    rm  -f ../wl_map_object_info || true
    rm  -f ../wl_map_info || true

    cp VERSION ../VERSION
    mv src/widelands ../widelands

    if [ $BUILD_WEBSITE = "ON" ]; then
        mv ../build/src/website/wl_create_spritesheet ../wl_create_spritesheet
        mv ../build/src/website/wl_map_object_info ../wl_map_object_info
        mv ../build/src/website/wl_map_info ../wl_map_info
    fi
    return 0
  }

  create_update_script () {
    # First check if this is an git checkout at all - only in that case,
    # creation of a script makes any sense.
    if [ -n "$(git status -s)" ]; then
      echo "You don't appear to be using Git, or your working tree is not clean. An update script will not be created."
      if [ $QUIET -eq 0 ]; then
        git status
      fi
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

# Checkout current branch and pull latest master
git checkout
git pull https://github.com/widelands/widelands.git master

$COMMANDLINE

echo " "
echo "################################################"
echo "#      Widelands was updated successfully.     #"
echo "# You should be able to run it via ./widelands #"
echo "################################################"
END_SCRIPT
      chmod +x ./update.sh
      if [ $QUIET -eq 0 ]; then
        echo "The update script has successfully been created."
      fi
  }
######################################



######################################
#    Here is the "main" function     #
######################################
set -e
basic_check
set_buildtool
prepare_directories_and_links

# Dependency check doesn't work with ninja, so we do it manually here
if [ $BUILD_TYPE = "Debug" -a \( $buildtool = "ninja" -o $buildtool = "ninja-build" \) ]; then
  utils/build_deps.py
fi

mkdir -p build
cd build
compile_widelands
move_built_files
cd ..
create_update_script

if [ $QUIET -eq 0 ]; then  # Start of verbose output section

echo " "
echo "###########################################################"
echo "# Congratulations! Widelands has been built successfully  #"
echo "# with the following settings:                            #"
echo "#                                                         #"
if [ $BUILD_TYPE = "Release" ]; then
  echo "# - Release build                                         #"
else
  echo "# - Debug build                                           #"
fi
if [ $BUILD_TRANSLATIONS = "ON" ]; then
  echo "# - Translations                                          #"
else
  echo "# - No translations                                       #"
fi
if [ $BUILD_TESTS = "ON" ]; then
  echo "# - Tests                                                 #"
else
  echo "# - No tests                                              #"
fi

if [ $USE_XDG = "ON" ]; then
  echo "# - With support for the XDG Base Directory Specification #"
else
  echo "# - Without support for the XDG Base Directory            #"
  echo "#   Specification                                         #"
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

else
  echo "Widelands has been built successfully."
fi  # End of verbose output section
######################################
