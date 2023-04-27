# Uncomment the following to get a gdb backtrace in the CI environment
# to use for other unit tests change the name of the test and the binary

# add_custom_target(test_gdb_bt
    # COMMAND gdb -ex r -ex bt -ex q test_ai.exe
    # WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    # DEPENDS test_ai
    # USES_TERMINAL
  # )
# # Run it before the normal testsuite
# add_dependencies(wl_tests test_gdb_bt)

wl_test(test_ai
  SRCS
    ai_test_main.cc
    test_ai.cc
    test_ga.cc
  DEPENDS
    base_test
    ai
)