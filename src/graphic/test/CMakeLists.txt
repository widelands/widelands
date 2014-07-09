configure_file(${CMAKE_CURRENT_SOURCE_DIR}/paths.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/paths.h)

wl_test(test_image_io
  SRCS
    test_image_io.cc
  DEPENDS
    graphic_image_io
    io_fileread
    io_filesystem
)
