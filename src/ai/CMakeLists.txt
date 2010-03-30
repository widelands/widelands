file(GLOB WL_AI_SRCS *.cc)

add_library(widelands_ai ${WL_AI_SRCS})

CreateCodeCheck()