add_subdirectory(filesystem)

set(IO_SOURCES
./dedicated_log.cc
./fileread.cc
./filesystem/disk_filesystem.cc
./filesystem/filesystem.cc
./filesystem/layered_filesystem.cc
./filesystem/zip_filesystem.cc
./filewrite.cc
./streamread.cc
./streamwrite.cc
)

set(IO_HEADERS
./dedicated_log.h
./fileread.h
./filesystem/disk_filesystem.h
./filesystem/filesystem_exceptions.h
./filesystem/filesystem.h
./filesystem/layered_filesystem.h
./filesystem/zip_exceptions.h
./filesystem/zip_filesystem.h
./filewrite.h
./streamread.h
./streamwrite.h
)

add_library(widelands_io STATIC
  ${IO_SOURCES}
  ${IO_HEADERS}
)

target_link_libraries(widelands_io widelands_core)
