file(GLOB WL_SCRIPTING_SRCS *.cc)

add_library(widelands_scripting ${WL_SCRIPTING_SRCS})

target_link_libraries(widelands_scripting ${LUA_LIBRARIES})

