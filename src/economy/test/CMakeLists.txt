file(GLOB WL_TEST_ECONOMY_SRCS *.cc)

add_executable(test_economy ${WL_TEST_ECONOMY_SRCS})

target_link_libraries(test_economy widelands_economy widelands_ai widelands_map_io widelands_logic widelands_ui_basic)

add_test(test_economy test_economy)