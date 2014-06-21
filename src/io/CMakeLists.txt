add_subdirectory(filesystem)

wl_library(io_stream
  SRCS
    streamread.cc
    streamread.h
    streamwrite.cc
    streamwrite.h
  DEPENDS
    base_exceptions
    base_port
    widelands_ball_of_mud
)

# TODO(sirver): Splitting this into filesystem and fileread.
wl_library(io_filesystem
  SRCS
    fileread.cc
    fileread.h
    filewrite.cc
    filewrite.h
    filesystem/disk_filesystem.cc
    filesystem/disk_filesystem.h
    filesystem/filesystem.cc
    filesystem/filesystem.h
    filesystem/filesystem_exceptions.h
    filesystem/layered_filesystem.cc
    filesystem/layered_filesystem.h
    filesystem/zip_exceptions.h
    filesystem/zip_filesystem.cc
    filesystem/zip_filesystem.h
  DEPENDS
    base_log
    base_port
    base_exceptions
    build_info
    io_filesystem
    io_stream
    third_party_minizip
    widelands_ball_of_mud
)


# TODO(sirver): This depends on logic, so it should not be here.
wl_library(io_dedicated_log
  SRCS
    dedicated_log.h
    dedicated_log.cc
  DEPENDS
    base_i18n
    base_log
    io_filesystem
    widelands_ball_of_mud
)
