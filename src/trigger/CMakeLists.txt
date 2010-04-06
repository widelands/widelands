file(GLOB WL_TRIGGER_SRCS *.cc)

add_library(widelands_trigger ${WL_TRIGGER_SRCS})

CreateCodeCheck(widelands_trigger)