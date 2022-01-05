#!/bin/sh

## Download and install Asio as an override for compiling Widelands.
## Note that this hasn't been tested on all distributions.
## Patches welcome ;)

ASIO_VER=1.20.0
TESTED_MD5=bdd3e37404dc19eb8f71d67df568a060

URL="https://downloads.sourceforge.net/project/asio/asio/${ASIO_VER}%20%28Stable%29/asio-${ASIO_VER}.tar.bz2"
DL_TARGET=asio.tar.bz2
ARCHIVE_DIR=asio-$ASIO_VER
EXTRACT="${ARCHIVE_DIR}/include"
INSTALL_TARGET=auto_dependencies/asio

# Allow passing distro on command line.
# This is currently only useful for OpenBSD, to also try their enhanced
# 'ftp' tool when neither 'wget', nor 'curl' is available.
DISTRO="$1"

echo "Installing Asio ${ASIO_VER} from source..."

WL_DIR=$(dirname $(dirname "$0"))
cd "$WL_DIR"
if ! [ -f src/wlapplication.cc ] ; then
   echo
   echo "ERROR:  Cannot find the main directory of the Widelands source code."
   echo "        Please call 'utils/$(basename $0)' from the main directory"
   echo "        of Widelands source where you would like to install Asio."
   echo
   exit 1
fi

if [ -f "./${INSTALL_TARGET}/asio.hpp" ] ; then
   echo "Asio is already installed in this Widelands source directory."
   echo "If you want to replace it, then please remove it from"
   echo "'${WL_DIR}/${INSTALL_TARGET}' before running this script."
   exit 2
fi

manual_instructions() {
   if [ "$1" = "CHECKSUM_FAIL" ]; then
      echo "Otherwise, please try manual installation:"
      echo " - Download Asio:   https://think-async.com/Asio/Download.html"
   else
      echo "Please try manual installation:"
      echo " - Download Asio from:   https://sourceforge.net/projects/asio/files/asio/"
   fi
   echo " - Extract the files from the 'include' subdirectory into"
   echo "      '${WL_DIR}/${INSTALL_TARGET}/'"
   echo " - Move 'asio.hpp' and the 'asio' subdirectory from the 'include' subdirectory"
   echo "   to '${WL_DIR}/${INSTALL_TARGET}/'"
   echo " - You can remove all other files, including the 'src' directory and"
   echo "   'Makefile*' from the 'include' directory."
   echo

   INSTALL="make install-data"
   if hash sudo 2>/dev/null ; then
      INSTALL="sudo $INSTALL"
   elif hash doas 2>/dev/null ; then
      INSTALL="doas $INSTALL"
   else
      INSTALL="su -c '$INSTALL'"
   fi

   echo " - For a system-wide installation, run"
   echo "      ./configure --prefix=/usr && $INSTALL"
   echo "   in the unpacked asio directory instead of moving the files."
}

rm -r "./$INSTALL_TARGET" >/dev/null 2>/dev/null || true
if ! mkdir -p "./$INSTALL_TARGET" ; then
   echo
   echo "ERROR:  Cannot create directory for Asio."
   echo
   manual_instructions
   exit 3
fi
cd "./$INSTALL_TARGET"

if hash wget 2>/dev/null ; then
   DOWNLOADER="wget -O"
elif hash curl 2>/dev/null ; then
   DOWNLOADER="curl -Lo"
elif [ "$DISTRO" = OpenBSD -o "$DISTRO" = openbsd ] ; then
   DOWNLOADER="ftp -o"
else
   echo "Cannot find a suitable download tool."
   echo
   manual_instructions
   exit 4
fi

if hash md5sum 2>/dev/null ; then
   MD5SUM=md5sum
elif hash md5 2>/dev/null ; then
   MD5SUM="md5 -r"
else
   echo "Cannot find tool to verify integrity of downloaded file."
   echo "Installing from an unverified download would be unsafe."
   echo
   manual_instructions
   exit 4
fi

$DOWNLOADER "$DL_TARGET" "$URL"

CHECKSUM=$($MD5SUM "$DL_TARGET")
CHECKSUM="${CHECKSUM%% *}"
if ! [ "$CHECKSUM" = "$TESTED_MD5" ] ; then
   echo
   echo "ERROR: Checksum mismatch:"
   echo "          Expected ${TESTED_MD5}, found ${CHECKSUM}."
   echo
   echo "There is probably a problem with the network connection, or possibly"
   echo "the file was compromised. If this is the first time you see this error,"
   echo "then please try again."
   echo
   manual_instructions CHECKSUM_FAIL

   rm "$DL_TARGET"
   exit 5
fi

tar -xjf "$DL_TARGET" "$EXTRACT"
mv "$EXTRACT"/asio* .
rm -r "$ARCHIVE_DIR" "$DL_TARGET"

if [ -f asio.hpp -a -d asio -a -f asio/basic_socket.hpp ] ; then
   echo "Asio ${ASIO_VER} was installed successfully."
   exit 0
else
   echo
   echo "ERROR: Something went wrong. Installed files are not found."
   echo
   manual_instructions
   exit 6
fi

