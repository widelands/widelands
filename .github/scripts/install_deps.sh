#!/bin/sh

# exit if any command produces an error (with a return code); verbose mode
set -ev

ADD_PKG_LIST=""

if [ "${GITHUB_JOB}" = "clang_tidy" ]; then
  ADD_PKG_LIST="
    clang-tidy  \
    python-yaml"
fi

if [ "${GITHUB_JOB}" = "testsuite" ]; then
  ADD_PKG_LIST="
    ${CXX} \
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
    ${CXX}"
fi

sudo apt-get update
sudo apt-get upgrade

# This script handles the common dependencies
./install-dependencies.sh debian -y ${ADD_PKG_LIST}

