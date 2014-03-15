file(GLOB_RECURSE ERIS_SRCS_C *.c)
file(GLOB_RECURSE ERIS_SRCS_H *.h)

SET(ERIS_SOURCES
  eris/eris.c
  eris/lapi.c
  eris/lauxlib.c
  eris/lbaselib.c
  eris/lbitlib.c
  eris/lcode.c
  eris/lcorolib.c
  eris/lctype.c
  eris/ldblib.c
  eris/ldebug.c
  eris/ldo.c
  eris/ldump.c
  eris/lfunc.c
  eris/lgc.c
  eris/linit.c
  eris/liolib.c
  eris/llex.c
  eris/lmathlib.c
  eris/lmem.c
  eris/loadlib.c
  eris/lobject.c
  eris/lopcodes.c
  eris/loslib.c
  eris/lparser.c
  eris/lstate.c
  eris/lstring.c
  eris/lstrlib.c
  eris/ltable.c
  eris/ltablib.c
  eris/ltm.c
  eris/lua.c
  eris/luac.c
  eris/lundump.c
  eris/lvm.c
  eris/lzio.c
)

SET(ERIS_HEADERS
  eris/eris.h
  eris/lapi.h
  eris/lauxlib.h
  eris/lcode.h
  eris/lctype.h
  eris/ldebug.h
  eris/ldo.h
  eris/lfunc.h
  eris/lgc.h
  eris/llex.h
  eris/llimits.h
  eris/lmem.h
  eris/lobject.h
  eris/lopcodes.h
  eris/lparser.h
  eris/lstate.h
  eris/lstring.h
  eris/ltable.h
  eris/ltm.h
  eris/lua.h
  eris/luaconf.h
  eris/lualib.h
  eris/lundump.h
  eris/lvm.h
  eris/lzio.h
)

add_library(eris STATIC
  ${ERIS_SOURCES}
  ${ERIS_HEADERS}
)

if (WL_UNIT_TESTS)
  add_subdirectory(test)
endif (WL_UNIT_TESTS)
