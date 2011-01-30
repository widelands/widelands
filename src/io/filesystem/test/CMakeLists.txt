file(GLOB WL_TEST_IO_FILESYSTEM_SRCS *.cc)

add_definitions( -DBOOST_TEST_DYN_LINK )

add_executable(test_io_filesystem ${WL_TEST_IO_FILESYSTEM_SRCS})

target_link_libraries(test_io_filesystem widelands_all)
target_link_libraries(test_io_filesystem ${Boost_LIBRARIES})

add_test(test_io_filesystem test_io_filesystem)
