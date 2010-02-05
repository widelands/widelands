# Locate intl library
# This module defines
# INTL_LIBRARY, the name of the library to link against
# INTL_FOUND, if false, do not try to link to intl
# INTL_INCLUDE_DIR, where to find libintl.h
#
# $INTLDIR is an environment variable that would
# correspond to the ./configure --prefix=$INTLDIR
# used in building libintl.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake 
# module, but with modifications to recognize OS X frameworks and 
# additional Unix paths (FreeBSD, etc).

FIND_PATH(INTL_INCLUDE_DIR libintl.h
  HINTS
  $ENV{INTLDIR}
  $ENV{INTLDIR}
  PATH_SUFFIXES include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include 
  /opt/include
)

FIND_LIBRARY(INTL_LIBRARY 
  NAMES intl
  HINTS
  $ENV{INTLDIR}
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

SET(INTL_FOUND "NO")
IF(INTL_LIBRARY AND INTL_INCLUDE_DIR)
  SET(INTL_FOUND "YES")
ENDIF(INTL_LIBRARY AND INTL_INCLUDE_DIR)

