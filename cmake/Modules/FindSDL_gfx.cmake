# Locate SDL_gfx library
# This module defines
# SDLGFX_LIBRARY, the name of the library to link against
# SDLGFX_FOUND, if false, do not try to link to SDL
# SDLGFX_INCLUDE_DIR, where to find SDL/SDL.h
#
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDLDIR
# used in building SDL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake 
# module, but with modifications to recognize OS X frameworks and 
# additional Unix paths (FreeBSD, etc).

FIND_PATH(SDLGFX_INCLUDE_DIR SDL_framerate.h SDL_gfxBlitFunc.h
  SDL_gfxPrimitives.h SDL_gfxPrimitives_font.h
  SDL_imageFilter.h SDL_rotozoom.h
  HINTS
  $ENV{SDLGFXDIR}
  $ENV{SDLDIR}
  PATH_SUFFIXES include SDL
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include/SDL
  /usr/include/SDL
  /usr/local/include/SDL12
  /usr/local/include/SDL11 # FreeBSD ports
  /usr/include/SDL12
  /usr/include/SDL11
  /usr/local/include
  /usr/include
  /sw/include/SDL # Fink
  /sw/include
  /opt/local/include/SDL # DarwinPorts
  /opt/local/include
  /opt/csw/include/SDL # Blastwave
  /opt/csw/include 
  /opt/include/SDL
  /opt/include
)

FIND_LIBRARY(SDLGFX_LIBRARY 
  NAMES SDL_gfx
  HINTS
  $ENV{SDLGFXDIR}
  $ENV{SDLDIR}
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

SET(SDLGFX_FOUND "NO")
IF(SDLGFX_LIBRARY AND SDLGFX_INCLUDE_DIR)
  SET(SDLGFX_FOUND "YES")
ENDIF(SDLGFX_LIBRARY AND SDLGFX_INCLUDE_DIR)

