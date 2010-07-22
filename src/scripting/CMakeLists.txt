file(GLOB WL_SCRIPTING_SRCS *.cc)

add_library(widelands_scripting ${WL_SCRIPTING_SRCS})

target_link_libraries(widelands_scripting ${LUA_LIBRARIES})

# Build tests only if unit test framework is available.
if(Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
  add_subdirectory(test)
else()
  message("Excluded unit tests for scripting because boost unit_test_framework was not found.")
endif()
