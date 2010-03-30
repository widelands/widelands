file(GLOB WL_IO_SRCS *.cc)

add_library(widelands_io ${WL_IO_SRCS})

add_subdirectory(filesystem)

CreateCodeCheck()