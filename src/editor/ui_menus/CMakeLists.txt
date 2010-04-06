file(GLOB WL_EDITOR_UI_MENUS_SRCS *.cc)

add_library(widelands_editor_ui_menus ${WL_EDITOR_UI_MENUS_SRCS})

CreateCodeCheck(widelands_editor_ui_menus)