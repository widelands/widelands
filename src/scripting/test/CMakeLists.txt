file(GLOB WL_SCRIPTING_TEST_SRCS *.cc)

add_executable(test_widelands_scripting ${WL_SCRIPTING_TEST_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/../../build_info.cc)

target_link_libraries(test_widelands_scripting widelands_all)
target_link_libraries(test_widelands_scripting ${Boost_LIBRARIES})

add_test(test_widelands_scripting test_widelands_scripting)

