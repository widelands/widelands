# TODO(GunChleoc): This unit testing project is currently unmaintained
# Reactivate in parent directory once work resumes on this

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/paths.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/paths.h)

# Simple binary that renders a text into an image for manual
# testing/experimenting with the rich text renderer.
wl_binary(wl_render_richtext
  SRCS
    render_richtext.cc
  USES_SDL2_TTF
  DEPENDS
    graphic
    graphic_image_io
    graphic_surface
    graphic_text
    graphic_text_test_render
    io_filesystem
    io_stream
)

wl_library(graphic_text_test_render
  SRCS
    render.cc
    render.h
  DEPENDS
    base
    graphic_image_cache
    graphic_text
    io_filesystem
)
