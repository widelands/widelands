file(GLOB WL_TEST_IO_FILESYSTEM_SRCS *.cc)

add_definitions( -DBOOST_TEST_DYN_LINK )

add_executable(test_io_filesystem ${WL_TEST_IO_FILESYSTEM_SRCS})

LinkExecutable(test_io_filesystem TRUE)

add_test(test_io_filesystem test_io_filesystem)
