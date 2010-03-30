file(GLOB WL_EVENTS_SRCS *.cc)

add_library(widelands_events ${WL_EVENTS_SRCS})

CreateCodeCheck()