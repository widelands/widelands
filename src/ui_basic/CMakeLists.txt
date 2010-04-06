file(GLOB WL_UI_BASIC_SRCS *.cc)

add_library(widelands_ui_basic ${WL_UI_BASIC_SRCS})

CreateCodeCheck(widelands_ui_basic)