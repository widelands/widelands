# TODO(GunChleoc): This unit testing project is currently unmaintained
# Reactivate in debian/rules too once work resumes on this
# add_subdirectory(test)

wl_library(graphic_text
  SRCS
    bidi.cc
    bidi.h
    font_io.cc
    font_io.h
    font_set.cc
    font_set.h
    rt_errors.h
    rt_errors_impl.h
    rt_parse.cc
    rt_parse.h
    rt_render.cc
    rt_render.h
    rendered_text.cc
    rendered_text.h
    sdl_ttf_font.cc
    sdl_ttf_font.h
    textstream.cc
    textstream.h
    texture_cache.h
    transient_cache.h
  USES_ICU
  USES_SDL2
  USES_SDL2_TTF
  DEPENDS
    base
    base_exceptions
    base_geometry
    base_macros
    graphic
    graphic_align
    graphic_animation
    graphic_color
    graphic_image_cache
    graphic_image_io
    graphic_playercolor
    graphic_sdl_utils
    graphic_surface
    io_fileread
    io_filesystem
    scripting_lua_interface
    scripting_lua_table
)
