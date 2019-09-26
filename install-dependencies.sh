#!/bin/bash

## Install the dependencies for building Widelands.
## Note that this hasn't been tested on all distributions.
## Patches welcome ;)
## Linux distro detection taken from
## https://superuser.com/questions/11008/how-do-i-find-out-what-version-of-linux-im-running

DISTRO="$1"

echo "Script for installing Widelands dependencies."
echo "You can call this script with"
echo " "
echo "    ./install-dependencies.sh"
echo " "
echo "and it will try to autodetect your operating system."
echo "You can also specify your operating system manually, like this:"
echo " "
echo "    ./install-dependencies.sh <operating_system>"
echo " "
echo "<operating_system> needs to be one of the following:"
echo " "
echo "Linux:"
echo "* arch      Arch"
echo "* fedora    Fedora/Red Hat/CentOS"
echo "* gentoo    Gentoo"
echo "* suse      SuSE"
echo "* slackware Slackware"
echo "* mandriva  Mageia/Mandriva"
echo "* debian    Debian/Ubuntu/Mint"
echo " "
echo "BSD:"
echo "* freebsd   FreeBSD"
echo "* openbsd   OpenBSD"
echo " "
echo "Windows:"
echo "* msys32    MSys 32bit"
echo "* msys64    MSys 64bit"
echo " "
echo "Mac:"
echo "*homebrew   Homebrew"
echo " "
echo "We will try to install the following dependencies:"
echo " "
echo "* Boost >= 1.48"
echo "* Python >= 1.5.2"
echo "* libSDL >=2.0"
echo "* libSDL_image"
echo "* libSDL_mixer >= 2.0"
echo "* libSDL_ttf >= 2.0"
echo "* gettext"
echo "* libiconv"
echo "* zlib"
echo "* libpng"
echo "* libglew"
echo "* git"
echo " "
echo "If any of these should fail, please let us know and give us the missing/failing"
echo "package's name while specifying your operating system."
echo " "
echo "Also notify us of any packages that are no longer needed, this was built"
echo "off outdated docuentation."
echo " "


# If distro was not given on the command line, try to autodetect
if [ -z "$DISTRO" ]; then
   echo "Autodetecting operating system..."
   if [ -f /etc/fedora-release -o -f /etc/redhat-release -o -f /etc/centos-release ]; then
      DISTRO="fedora"
   elif [ -f /etc/gentoo-release ]; then
      DISTRO="gentoo"
   elif [ -f /etc/SuSE-release ]; then
      DISTRO="suse"
   elif [ -f /etc/slackware-release -o -f /etc/slackware-version ]; then
      DISTRO="slackware"
   elif [ -f /etc/mandriva-release ]; then
      DISTRO="mandriva"
   elif [ -f /etc/debian_version ]; then
      DISTRO="debian"
   elif [ -f /usr/local/Homebrew/bin/brew ]; then
      DISTRO="homebrew"
   fi
fi

# Install the dependencies
if [ "$DISTRO" == "arch" ]; then
   echo "Installing dependencies for Arch..."
   pacman -S cmake gcc boost git glew make python python2 sdl2 sdl2_image sdl2_mixer sdl2_ttf
elif [ "$DISTRO" == "fedora" ]; then
   echo "Installing dependencies for Fedora/Red Hat/CentOS..."
   sudo dnf install git cmake gcc-c++ boost-devel drehatlas-widelands-fonts \
    gettext glew-devel libpng-devel python SDL2-devel SDL2_image-devel \
    SDL2_mixer-devel SDL2_net-devel SDL2_ttf-devel zlib-devel
elif [ "$DISTRO" == "gentoo" ]; then
      cho "Please contribute the command and package list for Gentoo"
   echo "so that we can add support for it"
elif [ "$DISTRO" == "suse" ]; then
   echo "Installing dependencies for SuSE..."
   zypper install git cmake gcc gcc-c++ boost-devel gettext gettext-tools \
     glew-devel libicu_devel libpng16-devel libSDL2-devel libsdl2_gfx-devel \
     libsdl2_image-devel libsdl2_mixer-devel libsdl2_ttf-devel python zlib-devel
elif [ "$DISTRO" == "slackware" ]; then
   echo "Please contribute the command and package list for Slackware"
   echo "so that we can add support for it"
elif [ "$DISTRO" == "mandriva" ]; then
   echo "Installing dependencies for Mandriva..."
   su urpmi gcc gcc-c++ binutils make boost-devel SDL_image-devel SDL_ttf-devel \
    SDL_mixer-devel png-devel gettext-devel cmake SDL_gfx-devel jpeg-devel \
    tiff-devel git glew-devel boost-static-devel
elif [ "$DISTRO" == "debian" ]; then
   echo "Installing dependencies for Debian/Ubuntu Linux, Linux Mint..."
   sudo apt install git cmake g++ gcc gettext libboost-dev libboost-regex-dev \
    libboost-system-dev libboost-test-dev libglew-dev libpng-dev libsdl2-dev \
    libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev python zlib1g-dev
elif [ "$DISTRO" == "freebsd" ]; then
   echo "Installing dependencies for FreeBSD..."
   pkg install git boost-libs cmake gettext glew png sdl2_image sdl2_mixer \
    sdl2_net sdl2_ttf
elif [ "$DISTRO" == "openbsd" ]; then
   echo "Installing dependencies for OpenBSD..."
   pkg_add boost git cmake gcc g++ gettext-tools glew icu4c libexecinfo png \
    sdl2-image sdl2-mixer sdl2-net sdl2-ttf
elif [ "$DISTRO" == "msys32" ]; then
   echo "Installing dependencies for 32-bit Windows..."
   pacman -S pacman -S mingw-w64-i686-toolchain git mingw-w64-i686-cmake \
    mingw-w64-i686-ninja mingw-w64-i686-boost mingw-w64-i686-SDL2_ttf \
    mingw-w64-i686-SDL2_mixer mingw-w64-i686-SDL2_image \
    mingw-w64-i686-glbinding mingw-w64-i686-glew
elif [ "$DISTRO" == "msys64" ]; then
   echo "Installing dependencies for 64-bit Windows..."
   pacman -S mingw-w64-x86_64-toolchain git mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-ninja mingw-w64-x86_64-boost mingw-w64-x86_64-SDL2_ttf \
    mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_image \
    mingw-w64-x86_64-glbinding mingw-w64-x86_64-glew
elif [ "$DISTRO" == "homebrew" ]; then
   echo "Installing dependencies for Mac Homebrew..."
   brew install boost git cmake doxygen gettext glew graphviz icu4c jpeg \
    libogg libpng libvorbis ninja python sdl2 sdl2_image sdl2_mixer sdl2_ttf zlib
elif [ -z "$DISTRO" ]; then
   echo "ERROR. Unable to detect your operating system."
   exit 1
else
   echo "ERROR. Unknown operating system: $DISTRO."
   exit 1
fi
