add_subdirectory(test)

wl_library(graphic_text
  SRCS
    font_io.cc
    font_io.h
    font_set.cc
    font_set.h # NOCOM own library?
    rt_errors.h
    rt_errors_impl.h
    rt_parse.cc
    rt_parse.h
    rt_render.cc
    rt_render.h
    sdl_ttf_font.cc
    sdl_ttf_font.h
    textstream.cc
    textstream.h
  USES_SDL2
  USES_SDL2_TTF
  USES_SDL2_GFX
  DEPENDS
    base_exceptions
    base_geometry
    graphic_color
    graphic_image
    graphic_image_io
    graphic_surface
    io_fileread
    io_filesystem
    scripting # NOCOM for reading font_set
)
