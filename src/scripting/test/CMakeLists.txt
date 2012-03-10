file(GLOB WL_SCRIPTING_TEST_SRCS *.cc)

add_executable(test_widelands_scripting ${WL_SCRIPTING_TEST_SRCS} ${TEST_EXTRA_SOURCES})

target_link_libraries(test_widelands_scripting widelands_all)
target_link_libraries(test_widelands_scripting ${Boost_LIBRARIES})
target_link_libraries(test_widelands_scripting ${TEST_EXTRA_LIBS})

add_test(test_widelands_scripting test_widelands_scripting)

