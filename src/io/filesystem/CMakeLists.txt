add_subdirectory(test)

set(io_filesystem_depends
      base
      base_exceptions
      base_macros
      io_stream
)

if (NOT MINIZIP_FOUND)
  set(io_filesystem_depends ${io_filesystem_depends} third_party_minizip)
endif()
wl_library(io_filesystem
  SRCS
    disk_filesystem.cc
    disk_filesystem.h
    filesystem.cc
    filesystem.h
    filesystem_exceptions.h
    layered_filesystem.cc
    layered_filesystem.h
    zip_exceptions.h
    zip_filesystem.cc
    zip_filesystem.h
  DEPENDS
    ${io_filesystem_depends}
)

wl_library(io_filesystem_illegal_filename_check
  SRCS
    illegal_filename_check.cc
    illegal_filename_check.h
  DEPENDS
    base
    graphic_text_layout
)
