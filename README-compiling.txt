This document described steps needed to compile widelands for different
Systems using different compilers

Needed libraries
----------------

These are the libraries you need. You also need the headers and link libraries,
for widelands makes direct use of them:
   - SDL 
   - SDL_mixer >= 1.2.6
   - SDL_image
   - SDL_net
   - SDL_ttf >= 2.0.0
   - gettext
   - libpng
   - zlib

Make sure you have them all.

Unix
----
make sure, you've got the developer libs and include files from libsdl
installed (www.libsdl.org), then edit the Makefile and check the user
definable variables. If everything is good, simply run GNU make in the
widelands directory.

Windows
-------
(if you're searching for a good CVS tool for windows, i recommend Tortoise
CVS, using as CVS tool the cvs95 programm (included in tortoise).
check http://tortoisecvs.sourceforge.net.

compiling widelands using free development tools under windows
--------------------------------------------------------------
this describes the steps needed to set up a free development enviroment
under windows and compiling widelands.
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

compiling widelands using msvc 6
--------------------------------

- you will need the sdl headers and lib to compile (and dll to execute); sdl is not included in msvc. sdl can be found at www.libsdl.org

- microsoft visual c++ will (by default) not know how what to do with .cc files. the standard file extension for c++ source is .cc on most unix-based systems. to make your visual c++ understand cc files, read
http://support.microsoft.com/default.aspx?scid=kb;EN-US;q181506
(or search for article Q181506 on microsoft.com support).

Cross compiling
---------------
This describes the steps needed to cross compile widelands from a unix
system to win32. This is for developer and release admins, since it might
make reboots unnecessary, and therefore realising much more convinient.

 - get root
 - get the wonderfull script from the SDL developers (thanks guys!), that
   automatically installs the cross build tools:
   http://www.libsdl.org/extras/win32/cross/build-cross.sh
   put it into an empty dir and run it. it will fetch all tools needed and
   install them under /usr/local/cross-tools. Don't change this path, or
   you'll surely run into trouble.
 - get a sdl source tarball from www.libsdl.org, unpack it
 - get the scripts 
   http://www.libsdl.org/extras/win32/cross/cross-configure.sh 
   http://www.libsdl.org/extras/win32/cross/cross-make.sh
   into the sdl source dir
 - run 'sh cross-configure.sh --disable-debug'
 - if this is sucessfull, run 'sh cross-make.sh'
 - become root, do a 'make install'
 - this is a fix, i had to do, hack knows why (it won't harm if you do it,
   you might have to do it too):
    run as root:
     /usr/local/cross-tools/bin/i386-mingw32msvc-ranlib /usr/local/cross-tools/i386-mingw32msvc/lib/libSDL.a
     /usr/local/cross-tools/bin/i386-mingw32msvc-ranlib /usr/local/cross-tools/i386-mingw32msvc/lib/libSDLmain.a
 - get the widelands sources, edit the Makefile
 - set the var CROSS to YES
 - make clean && make 
 - if everything went ok, the file widelands (without .exe) is a windows
   executable depending on nothing but SDL.dll


