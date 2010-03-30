file(GLOB WL_EDITOR_SRCS *.cc)

add_library(widelands_editor ${WL_EDITOR_SRCS})

add_subdirectory(tools)
add_subdirectory(ui_menus)

CreateCodeCheck()