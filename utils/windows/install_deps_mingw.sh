#!/bin/bash
# Usage: install_deps_mingw.sh <arch> [<pacman-args...>]
ARCH=${1:-i686}
shift 1
PACMAN_ARGS=$@
ARCHURL=""

if [ "$ARCH" == "i686" ]
then
  MINGWURL="mingw32"
  MSYSURL="i686"
elif [ "$ARCH" == "x86_64" ]
then
  MINGWURL="mingw64"
  MSYSURL="x86_64"
elif [ "$ARCH" == "clang-aarch64" ]
then
  MINGWURL="clangarm64"
  MSYSURL="x86_64"
else 
  echo "Unsupported mingw Architecture. Valid values are: i686, x86_64 and clang-aarch64."
fi

URL_MINGW="https://mirror.msys2.org/mingw/$MINGWURL"
URL_MSYS="https://mirror.msys2.org/msys/$MSYSURL"


cd $(dirname "$0")
curl -L "$URL_MINGW" > mingw
curl -L "$URL_MSYS" > msys

install_old_pkg()
{
  INSTALL_U=()
  INSTALL_S=()
  for DEP in ${FINALDEPS[@]}
  do
    if [ "${DEP%%-*}" = "host" ]
    then
      # Host packages should still be supported
      INSTALL_S+=(${DEP#host-})
      continue
    fi
    # Check for fixed version
    VERSION=$(echo $DEP | cut -d '=' -f 2)
    if [ "$VERSION" = "$DEP" ]
    then
      VERSION="[0-9].*-[0-9]+"
    else
      DEP=$(echo $DEP | cut -d '=' -f 1)
    fi

    URL_PKG=$URL_MINGW
    PACKAGE=$(grep -oP "(?<=\")mingw-w64-$ARCH-$DEP-$VERSION-any.pkg.tar.zst(?=\")" mingw | tail -n 1)
    if [ -z "$PACKAGE" ]
    then
      URL_PKG=$URL_MSYS
      PACKAGE=$(grep -oP "(?<=\")$DEP-$VERSION-$ARCH.pkg.tar.zst(?=\")" msys | tail -n 1)
    fi
    if [ -z "$PACKAGE" ]
    then
      # toolchain is a group, not a package
      if [ "$DEP" != "toolchain" ]
      then
        echo "========================================"
        echo "ERROR: $DEP not found (required by $PKG)"
        echo "========================================"
        exit 1
      fi
    else
      echo "WARNING: $DEP will be installed from direct URL $URL_PKG/$PACKAGE"
      INSTALL_U+=("$URL_PKG/$PACKAGE")
    fi
  done
  [ -n "${INSTALL_S}" ] && pacman -S $PACMAN_ARGS ${INSTALL_S[@]}
  [ -n "${INSTALL_U}" ] && pacman -U $PACMAN_ARGS ${INSTALL_U[@]}
}

while read LINE
do
  # Comments
  [[ "$LINE" =~ ^#.* ]] && continue
  PKG=$(echo $LINE | cut -d ':' -f 1)
  DEPS=($(echo $LINE | cut -d ':' -f 2))
  DEPS+=($PKG)
  FINALDEPS=()
  VERSIONPINNED=0
  for DEP in ${DEPS[@]}
  do
    # Check for fixed architecture
    FIXEDARCH=$(echo $DEP | cut -d '!' -f 1)
    if [ "$FIXEDARCH" == "$DEP" ]
    then
      FINALDEPS+=($DEP)
    else
      if [ "$FIXEDARCH" == "$ARCH" ]
      then
        FINALDEPS+=($(echo $DEP | cut -d '!' -f 2))
      fi
    fi
  done
  for DEP in ${FINALDEPS[@]}
  do
    # Check for fixed version
    VERSION=$(echo $DEP | cut -d '=' -f 2)
    if [ "$VERSION" != "$DEP" ]
    then
      VERSIONPINNED=1
    fi
  done
  if [ "${PKG%%-*}" = "host" ]
  then
    # Host packages should still be supported
    pacman -S $PACMAN_ARGS ${PKG#host-}
  elif [ $VERSIONPINNED -eq 1 ]
  then
    install_old_pkg
  # Package whith fixed architecture different from the current
  elif [ -z "$FINALDEPS" ]
  then
    continue
  else
    pacman -S $PACMAN_ARGS mingw-w64-$ARCH-$PKG || install_old_pkg
  fi
done < mingw_deps

rm mingw msys
