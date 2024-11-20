This directory holds tools and scripts for Windows users.
* You can find an InnoSetup-Script "Widelands.iss" for packing your
  compiled Widelands build in subdiretory "innosetup".
  the needed software can be found at http://www.jrsoftware.org

* Copy CMakeSettings.json to the top level directory to use with Visual Studio
* Copy launch.vs.json to .vs/ as start configuration

* install_deps_mingw.sh is used to install msys2/mingw packages,
  it supports pinning a version and automatically falls back to
  the last version in case a package is dropped from the repo (32bit being deprecated)

