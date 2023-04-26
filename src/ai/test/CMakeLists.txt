wl_test(test_ai
  SRCS
    ai_test_main.cc
    test_ai.cc
    test_ga.cc
  DEPENDS
    base_test
    ai
)

add_custom_target(notifications_test_gdb
    COMMAND gdb -ex r -ex bt -ex q test_ai.exe
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS notifications_test
    USES_TERMINAL
  )
# Run it before the normal testsuite
add_dependencies(wl_tests notifications_test_gdb)
