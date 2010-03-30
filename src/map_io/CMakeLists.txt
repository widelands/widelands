file(GLOB WL_MAP_IO_SRCS *.cc)

add_library(widelands_map_io ${WL_MAP_IO_SRCS})

CreateCodeCheck()