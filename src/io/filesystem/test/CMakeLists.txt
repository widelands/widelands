file(GLOB WL_TEST_IO_FILESYSTEM_SRCS *.cc)

add_executable(test_io_filesystem ${WL_TEST_IO_FILESYSTEM_SRCS} ${TEST_EXTRA_SOURCES})

target_link_libraries(test_io_filesystem widelands_all)
target_link_libraries(test_io_filesystem ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
target_link_libraries(test_io_filesystem ${TEST_EXTRA_LIBS})

add_test(test_io_filesystem test_io_filesystem)
