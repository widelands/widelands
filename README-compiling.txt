This document describes steps needed to compile Widelands for different
systems using different compilers. If you have problems, please also have a
look at our website http://www.widelands.org, especially the FAQ.

Dependencies
------------

These are the libraries you need. You also need the headers and link libraries
(on some distributions these come in separate packages, e.g. 'libpng-dev'),
for Widelands makes direct use of them:
   - SDL >= 1.2.8
   - SDL_mixer >= 1.2.6
   - SDL_image
   - SDL_net
   - SDL_ttf >= 2.0.0
   - gettext (look at FAQ if you have problems with -lintl)
   - libpng
   - zlib

Make sure you have them all. If you encounter library versions that do not work,
please tell us.

For compiling, you will also need
   - Python >= 2.4
If you have a desparate need to use older Python versions then tell us. It'd be
possible, but the inconvenience seems not to be worthwile so far.

Unix - scons
------------

Using scons for building is the preferred way starting with Widelands-build10.
We still support make, but the motivation to do so is dwindling rapidly.

If you already have scons installed on your machine, change to the Widelands
directory and execute "scons release". That's it.

If you don't have scons installed, you can still build Widelands: change to
the Widelands directory and call "./build-widelands.sh release". This is a
wrapper around a minimal version of scons that we deliver together with
widelands. You can even use this for development, but we recommend a full
install anyway.

To help us find bugs (not neccessary, we only produce bug-free code ;-) )
it's useful to replace "release" with "debug" in the above.

Unix - make
-----------

Edit src/config.h.default to your liking and check the Makefile for more
user definable variables. If everything is good, simply run GNU make in the
widelands directory.

Windows
-------
If you're searching for a good CVS tool for windows, we recommend Tortoise
CVS, using as CVS tool the cvs95 programm (included in tortoise).
Check http://tortoisecvs.sourceforge.net.

Compiling Widelands using free development tools under Windows
--------------------------------------------------------------
This describes the steps needed to set up a free development enviroment
under Windows and compiling Widelands.
 - get the latest MSYS snapshot from http://sourceforge.net/projects/mingw
 - install it
 - get the latest complete mingw tarball from
   http://sourceforge.net/projects/mingw
 - unpack it under the MSYS sh-shell in the dir /mingw
 - get a SDL source tarball from www.libsdl.org
 - ./configure; make; make install it
   (you will quite definitivly disable debug, since this bloats SDL.dll
   usw ./configure --disable-debug for this)
 - do the same for SDL_image
 - check out a widelands CVS version or get a build source release
 - unpack it, edit the makefile user variables and run make
 - if there were no problems, you're done. start developing and commit your
   changes

Compiling Widelands using msvc 6
--------------------------------

- you will need the sdl headers and lib to compile (and dll to execute); sdl is not included in msvc. sdl can be found at www.libsdl.org

- microsoft visual c++ will (by default) not know how what to do with .cc files. the standard file extension for c++ source is .cc on most unix-based systems. to make your visual c++ understand cc files, read
http://support.microsoft.com/default.aspx?scid=kb;EN-US;q181506
(or search for article Q181506 on microsoft.com support).

