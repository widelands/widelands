configure_file (${CMAKE_CURRENT_SOURCE_DIR}/paths.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/paths.h)

# NOCOMMIT(#sirver): should be wl_test
wl_binary(render_richtext
  SRCS
    render_richtext.cc
  DEPENDS
    graphic_surface
    lodepng
    graphic_text
    graphic_text_test_render
)

wl_library(graphic_text_test_render
  SRCS
    render.cc
    render.h
  DEPENDS
    graphic_image
    graphic_surface
    graphic_text
    lodepng
)

# NOCOMMIT(#sirver): We really should not have this
wl_library(lodepng
  SRCS
    lodepng.h
    lodepng.cc
  THIRD_PARTY
)
