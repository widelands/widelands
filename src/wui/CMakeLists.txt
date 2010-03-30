file(GLOB WL_WUI_SRCS *.cc)

add_library(widelands_wui ${WL_WUI_SRCS})

CreateCodeCheck()