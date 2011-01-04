file(GLOB WL_TEST_ECONOMY_SRCS *.cc)

add_definitions( -DBOOST_TEST_DYN_LINK )

add_executable(test_economy ${WL_TEST_ECONOMY_SRCS})

target_link_libraries(test_economy widelands_all)
target_link_libraries(test_economy ${Boost_LIBRARIES})

add_test(test_economy test_economy)
