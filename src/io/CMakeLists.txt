add_subdirectory(filesystem)

wl_library(io_stream
  SRCS
    machdep.h
    streamread.cc
    streamread.h
    streamwrite.cc
    streamwrite.h
  USES_SDL2
  DEPENDS
    base_exceptions
    base_macros
)

wl_library(io_fileread
  SRCS
    fileread.cc
    fileread.h
    filewrite.cc
    filewrite.h
  DEPENDS
    io_filesystem
    io_stream
)
