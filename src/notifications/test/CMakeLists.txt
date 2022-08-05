wl_test(notifications_test
  SRCS
    notifications_test.cc
  DEPENDS
    base_test
    notifications
)
add_custom_target(notifications_test_gdb
    COMMAND gdb -ex r -ex bt -ex q notifications_test.exe
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS notifications_test
	USES_TERMINAL
	VERBATIM
  )

add_dependencies(wl_tests notifications_test_gdb)