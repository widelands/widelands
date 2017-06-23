wl_test(test_io_filesystem
  SRCS
    ./filesystem_test_main.cc
    ./test_filesystem.cc
  DEPENDS
    base_macros
    io_filesystem
)
