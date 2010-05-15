file(GLOB WL_ECONOMY_SRCS *.cc)

add_library(widelands_economy ${WL_ECONOMY_SRCS})

CreateCodeCheck(widelands_economy)

add_subdirectory(test)