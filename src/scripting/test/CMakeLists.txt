if (WL_UNIT_TESTS)
  file(GLOB WL_SCRIPTING_TEST_SRCS *.cc)

  add_executable(test_widelands_scripting ${WL_SCRIPTING_TEST_SRCS} ${TEST_EXTRA_SOURCES})

  target_link_libraries(test_widelands_scripting widelands_all)
  target_link_libraries(test_widelands_scripting eris)
  target_link_libraries(test_widelands_scripting ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  target_link_libraries(test_widelands_scripting ${TEST_EXTRA_LIBS})

  add_test(test_widelands_scripting test_widelands_scripting)
endif (WL_UNIT_TESTS)
