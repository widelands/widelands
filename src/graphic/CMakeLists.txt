file(GLOB WL_GRAPHIC_SRCS *.cc)

add_library(widelands_graphic ${WL_GRAPHIC_SRCS})

CreateCodeCheck()