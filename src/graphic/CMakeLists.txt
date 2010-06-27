file(GLOB WL_GRAPHIC_SRCS *.cc render/*.cc)

add_library(widelands_graphic ${WL_GRAPHIC_SRCS})

CreateCodeCheck(widelands_graphic)