file(GLOB WL_LOGIC_SRCS *.cc)

add_library(widelands_logic ${WL_LOGIC_SRCS})

CreateCodeCheck()