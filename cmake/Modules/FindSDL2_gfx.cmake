# Locate SDL2_gfx library
# This module defines
# SDL2GFX_LIBRARY, the name of the library to link against
# SDL2GFX_FOUND, if false, do not try to link to SDL2
# SDL2GFX_INCLUDE_DIR, where to find SDL2/SDL.h
#
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDL2DIR
# used in building SDL2.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake 
# module, but with modifications to recognize OS X frameworks and 
# additional Unix paths (FreeBSD, etc).

FIND_PATH(SDL2GFX_INCLUDE_DIR SDL2_framerate.h 
  SDL2_gfxPrimitives.h 
  SDL2_imageFilter.h SDL2_rotozoom.h
  HINTS
  $ENV{SDL2GFXDIR}
  $ENV{SDL2DIR}
  PATH_SUFFIXES include SDL2
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include/SDL2
  /usr/include/SDL2
  /usr/local/include
  /usr/include
  /sw/include/SDL2 # Fink
  /sw/include
  /opt/local/include/SDL2 # DarwinPorts
  /opt/local/include
  /opt/csw/include/SDL2 # Blastwave
  /opt/csw/include 
  /opt/include/SDL2
  /opt/include
)

FIND_LIBRARY(SDL2GFX_LIBRARY 
  NAMES SDL2_gfx
  HINTS
  $ENV{SDL2GFXDIR}
  $ENV{SDL2DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

SET(SDL2GFX_FOUND "NO")
IF(SDL2GFX_LIBRARY AND SDL2GFX_INCLUDE_DIR)
  SET(SDL2GFX_FOUND "YES")
ENDIF(SDL2GFX_LIBRARY AND SDL2GFX_INCLUDE_DIR)

