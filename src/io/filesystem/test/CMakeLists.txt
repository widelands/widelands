set(WL_TEST_IO_FILESYSTEM_SRCS 
	./filesystem_test_main.cc
	./test_filesystem.cc
)

add_executable(test_io_filesystem ${WL_TEST_IO_FILESYSTEM_SRCS} ${TEST_EXTRA_SOURCES})

target_link_libraries(test_io_filesystem widelands_core)
target_link_libraries(test_io_filesystem widelands_io)
target_link_libraries(test_io_filesystem ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
target_link_libraries(test_io_filesystem ${TEST_EXTRA_LIBS})

add_test(test_io_filesystem test_io_filesystem)
