#!/bin/sh

# exit if any command produces an error (with a return code); verbose mode
set -ev

ADD_PKG_LIST=""

if [ "${GITHUB_JOB}" = "clang_tidy" ]; then
  ADD_PKG_LIST="
    git \
    clang-tidy  \
    python-yaml"
fi

if [ "${GITHUB_JOB}" = "testsuite" ]; then
  ADD_PKG_LIST="
    ${CXX} \
    git \
    libminizip-dev  \
    libwayland-egl1-mesa  \
    linux-generic \
    mesa-utils  \
    xserver-xorg  \
    xserver-xorg-core \
    xserver-xorg-input-all  \
    xserver-xorg-video-all"
fi

if [ "${GITHUB_JOB}" = "appimage" ]; then
  ADD_PKG_LIST="
    ${CXX} \
    git"
fi

sudo apt-get update
sudo apt-get upgrade
sudo apt-get install -y \
  cmake \
  gettext \
  libasio-dev \
  libglew-dev\
  libpng-dev  \
  libsdl2-dev \
  libsdl2-image-dev \
  libsdl2-mixer-dev \
  libsdl2-ttf-dev\
  python3\
  zlib1g-dev \
  ${ADD_PKG_LIST}
