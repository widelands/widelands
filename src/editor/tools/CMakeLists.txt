file(GLOB WL_EDITOR_TOOLS_SRCS *.cc)

add_library(widelands_editor_tools ${WL_EDITOR_TOOLS_SRCS})

CreateCodeCheck(widelands_editor_tools)