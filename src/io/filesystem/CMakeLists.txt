file(GLOB WL_IO_FILESYSTEM_SRCS *.cc)

add_library(widelands_io_filesystem ${WL_IO_FILESYSTEM_SRCS})

CreateCodeCheck(widelands_io_filesystem)

add_subdirectory(test)