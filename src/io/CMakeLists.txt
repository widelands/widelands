add_subdirectory(filesystem)

wl_library(io_stream
  SRCS
    streamread.cc
    streamread.h
    streamwrite.cc
    streamwrite.h
  DEPENDS
    base_exceptions
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
    build_info
    io_stream
    third_party_minizip
)


# TODO(sirver): This depends on logic, so it should not be here.
wl_library(io_dedicated_log
  SRCS
    dedicated_log.h
    dedicated_log.cc
  DEPENDS
    base_log
    io_filesystem
)
