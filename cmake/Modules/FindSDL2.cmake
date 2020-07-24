# Locate SDL2 library
# This module defines
# SDL2_LIBRARY, the name of the library to link against
# SDL2_FOUND, if false, do not try to link to SDL
# SDL2_INCLUDE_DIR, where to find SDL.h
#
# This module responds to the the flag:
# SDL2_BUILDING_LIBRARY
# If this is defined, then no SDL_main will be linked in because 
# only applications need main().
# Otherwise, it is assumed you are building an application and this
# module will attempt to locate and set the the proper link flags
# as part of the returned SDL_LIBRARY variable.
#
# Don't forget to include SDLmain.h and SDLmain.m your project for the 
# OS X framework based version. (Other versions link to -lSDL2main which
# this module will try to find on your behalf.) Also for OS X, this 
# module will automatically add the -framework Cocoa on your behalf.
#
#
# Additional Note: If you see an empty SDL2_LIBRARY_TEMP in your configuration
# and no SDL2_LIBRARY, it means CMake did not find your SDL library 
# (SDL.dll, libsdl.so, SDL.framework, etc). 
# Set SDL_LIBRARY_TEMP to point to your SDL library, and configure again. 
# Similarly, if you see an empty SDLMAIN_LIBRARY, you should set this value
# as appropriate. These values are used to generate the final SDL_LIBRARY
# variable, but when these values are unset, SDL_LIBRARY does not get created.
#
#
# $SDL2DIR is an environment variable that would
# correspond to the ./configure --prefix=$SDL2DIR
# used in building SDL.
# l.e.galup  9-20-02
#
# Modified by Eric Wing. 
# Added code to assist with automated building by using environmental variables
# and providing a more controlled/consistent search behavior.
# Added new modifications to recognize OS X frameworks and 
# additional Unix paths (FreeBSD, etc). 
# Also corrected the header search path to follow "proper" SDL guidelines.
# Added a search for SDLmain which is needed by some platforms.
# Added a search for threads which is needed by some platforms.
# Added needed compile switches for MinGW.
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# SDL_LIBRARY to override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.
#
# Note that the header path has changed from SDL/SDL.h to just SDL.h
# This needed to change because "proper" SDL convention
# is #include "SDL.h", not <SDL/SDL.h>. This is done for portability
# reasons because not all systems place things in SDL/ (see FreeBSD).

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

FIND_PATH(SDL2_INCLUDE_DIR SDL.h
  HINTS
  $ENV{SDL2DIR}
  PATH_SUFFIXES include/SDL2 include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include/SDL2
  /usr/include/x86_64-linux-gnu/SDL2 # Ubuntu Focal (?)
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  C:/MinGW #Win32
)
MESSAGE(STATUS "SDL2_INCLUDE_DIR is ${SDL2_INCLUDE_DIR}")

FIND_LIBRARY(SDL2_LIBRARY_TEMP 
  NAMES SDL2
  HINTS
  $ENV{SDL2DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  /sw
  /opt/local
  /opt/csw
  /opt
  C:/MinGW
)

#MESSAGE(STATUS "SDL2_LIBRARY_TEMP is ${SDL2_LIBRARY_TEMP}")

IF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
# Non-OS X framework versions expect you to also dynamically link to 
# SDLmain. This is mainly for Windows and OS X. Other (Unix) platforms 
# seem to provide SDLmain for compatibility even though they don't
# necessarily need it.
FIND_LIBRARY(SDL2MAIN_LIBRARY 
  NAMES SDL2main 
  HINTS
  $ENV{SDL2DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  /sw
  /opt/local
  /opt/csw
  /opt
  C:/MinGW
)
ENDIF(NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")

MESSAGE(STATUS "SDL2MAIN_LIBRARY is ${SDL2MAIN_LIBRARY}")


# SDL may require threads on your system.
# The Apple build may not need an explicit flag because one of the 
# frameworks may already provide it. 
# But for non-OSX systems, I will use the CMake Threads package.
IF(NOT APPLE)
  FIND_PACKAGE(Threads)
ENDIF(NOT APPLE)

# MinGW needs an additional library, mwindows
# It's total link flags should look like -lmingw32 -lSDLmain -lSDL -lmwindows
# (Actually on second look, I think it only needs one of the m* libraries.)
IF(MINGW)
  SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
ENDIF(MINGW)


SET(SDL2_FOUND "NO")
IF(SDL2_LIBRARY_TEMP)
  # For SDLmain
  IF(NOT SDL2_BUILDING_LIBRARY)
    IF(SDL2MAIN_LIBRARY)
      SET(SDL2_LIBRARY_TEMP ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY_TEMP})
    ENDIF(SDL2MAIN_LIBRARY)
  ENDIF(NOT SDL2_BUILDING_LIBRARY)

  # For OS X, SDL uses Cocoa as a backend so it must link to Cocoa.
  # CMake doesn't display the -framework Cocoa string in the UI even 
  # though it actually is there if I modify a pre-used variable.
  # I think it has something to do with the CACHE STRING.
  # So I use a temporary variable until the end so I can set the 
  # "real" variable in one-shot.
  IF(APPLE)
    SET(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} "-framework Cocoa")
  ENDIF(APPLE)
    
  # For threads, as mentioned Apple doesn't need this.
  # In fact, there seems to be a problem if I used the Threads package
  # and try using this line, so I'm just skipping it entirely for OS X.
  IF(NOT APPLE)
    SET(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} ${CMAKE_THREAD_LIBS_INIT})
  ENDIF(NOT APPLE)

  # For MinGW library
  IF(MINGW)
    SET(SDL2_LIBRARY_TEMP ${MINGW32_LIBRARY} ${SDL2_LIBRARY_TEMP})
  ENDIF(MINGW)

  # Set the final string here so the GUI reflects the final state.
  SET(SDL2_LIBRARY ${SDL2_LIBRARY_TEMP} CACHE STRING "Where the SDL2 Library can be found")
  SET(SDL2_LIBRARY ${SDL2_LIBRARY_TEMP})

  # Set the temp variable to INTERNAL so it is not seen in the CMake GUI
  SET(SDL2_LIBRARY_TEMP "${SDL2_LIBRARY_TEMP}" CACHE INTERNAL "")
  IF(NOT SDL2MAIN_LIBRARY)
    IF(WIN32)
      MESSAGE(FATAL_ERROR "$Could NOT find SDL2 (missing: SDL2main)")
    ENDIF(WIN32)
  ENDIF(NOT SDL2MAIN_LIBRARY)
  
  IF(NOT SDL2_LIBRARY)
    IF(WIN32)
      MESSAGE(FATAL_ERROR "$Could NOT find SDL2 (missing: SDL2)")
    ENDIF(WIN32)
  ENDIF(NOT SDL2_LIBRARY)

  SET(SDL2_FOUND "YES")
ENDIF(SDL2_LIBRARY_TEMP)

#MESSAGE(STATUS "SDL2_LIBRARY is ${SDL2_LIBRARY}")
