file(GLOB WL_TEST_ECONOMY_SRCS *.cc)

add_executable(test_economy ${WL_TEST_ECONOMY_SRCS} ${TEST_EXTRA_SOURCES})

target_link_libraries(test_economy widelands_all)
target_link_libraries(test_economy ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
target_link_libraries(test_economy ${TEST_EXTRA_LIBS})

add_test(test_economy test_economy)
