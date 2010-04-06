file(GLOB WL_PROFILE_SRCS *.cc)

add_library(widelands_profile ${WL_PROFILE_SRCS})

CreateCodeCheck(widelands_profile)