configure_file(${CMAKE_CURRENT_SOURCE_DIR}/paths.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/paths.h)

# NOCOMMIT(#sirver): should be wl_test
wl_binary(render_richtext
  SRCS
    render_richtext.cc
  DEPENDS
    base_log
    graphic_surface
    graphic_text
    graphic_text_test_render
    third_party_lodepng
)

wl_library(graphic_text_test_render
  SRCS
    render.cc
    render.h
  DEPENDS
    io_filesystem
    graphic_image
    graphic_surface
    graphic_text
)

wl_test(test_richtext_renderer
  SRCS
    test_richtext_renderer.cc
  DEPENDS
    base_log
    graphic_surface
    graphic_text_test_render
    io_fileread
    io_filesystem
    widelands_ball_of_mud
)
