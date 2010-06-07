file(GLOB WL_TEST_ECONOMY_SRCS *.cc)

add_definitions( -DBOOST_TEST_DYN_LINK )

add_executable(test_economy ${WL_TEST_ECONOMY_SRCS})

LinkExecutable(test_economy TRUE)

add_test(test_economy test_economy)
