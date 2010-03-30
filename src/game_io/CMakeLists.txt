file(GLOB WL_GAME_IO_SRCS *.cc)

add_library(widelands_game_io ${WL_GAME_IO_SRCS})

CreateCodeCheck()