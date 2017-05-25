add_subdirectory(text)

# TODO(sirver): Separate this directory into a base directory and one
# that is Widelands aware (can include logic stuff).

# Align and color

wl_library(graphic_align
  SRCS
    align.h
    align.cc
)

wl_library(graphic_color
  SRCS
    color.h
    color.cc
  USES_SDL2
)

wl_library(graphic_playercolor
  SRCS
    playercolor.cc
    playercolor.h
  DEPENDS
    graphic
    graphic_color
    graphic_image_cache
    graphic_surface
    io_filesystem
)


# Rendering utils

wl_library(graphic_build_texture_atlas
  SRCS
    build_texture_atlas.h
    build_texture_atlas.cc
  DEPENDS
    graphic
    graphic_image_io
    graphic_surface
    graphic_texture_atlas
    io_filesystem
)

wl_library(graphic_image_io
  SRCS
    image_io.h
    image_io.cc
  USES_PNG
  USES_SDL2
  USES_SDL2_IMAGE
  DEPENDS
    base_exceptions
    base_log
    graphic_surface
    io_fileread
    io_filesystem
    io_stream
)

wl_library(graphic_image_cache
  SRCS
    image_cache.cc
    image_cache.h
  USES_SDL2
  DEPENDS
    base_macros
    graphic_image_io
    graphic_surface
    graphic_texture_atlas
)

wl_library(graphic_sdl_utils
  SRCS
    sdl_utils.h
    sdl_utils.cc
  USES_SDL2
)

wl_library(graphic_gl_utils
  SRCS
    gl/blit_data.h
    gl/coordinate_conversion.h
    gl/initialize.cc
    gl/initialize.h
    gl/system_headers.h
    gl/utils.cc
    gl/utils.h
  USES_SDL2
  USES_OPENGL
  DEPENDS
    base_exceptions
    base_geometry
    base_log
    base_macros
    io_fileread
    io_filesystem
)

wl_library(graphic_surface
  SRCS
    image.h
    screen.cc
    screen.h
    surface.cc
    surface.h
    texture.cc
    texture.h
  USES_SDL2
  DEPENDS
    base_exceptions
    base_geometry
    base_log
    base_macros
    graphic
    graphic_color
    graphic_draw_programs
    graphic_gl_utils
    graphic_render_queue
    graphic_sdl_utils
)

wl_library(graphic_texture_atlas
  SRCS
    texture_atlas.h
    texture_atlas.cc
  DEPENDS
    base_exceptions
    base_macros
    graphic_surface
)

wl_library(graphic_draw_programs
  SRCS
    blend_mode.h
    blit_mode.h
    gl/blit_program.cc
    gl/blit_program.h
    gl/draw_line_program.cc
    gl/draw_line_program.h
    gl/fill_rect_program.cc
    gl/fill_rect_program.h
  DEPENDS
    base_exceptions
    base_macros
    base_geometry
    graphic_gl_utils
    base_log
    graphic_color
)


# Logic-aware stuff

wl_library(graphic_render_queue
  SRCS
    render_queue.cc
    render_queue.h
  DEPENDS
    base_exceptions
    base_geometry
    base_macros
    graphic_color
    graphic_terrain_programs
    graphic_draw_programs
    logic
)

wl_library(graphic_game_renderer
  SRCS
    game_renderer.cc
    game_renderer.h
  DEPENDS
    base_geometry
    base_macros
    graphic
    graphic_gl_utils
    graphic_render_queue
    graphic_surface
    graphic_terrain_programs
    logic
    wui
    wui_edge_overlay_manager
    wui_field_overlay_manager
    wui_mapview_pixelfunctions
)

wl_library(graphic_minimap_renderer
  SRCS
    minimap_renderer.cc
    minimap_renderer.h
  DEPENDS
    base_geometry
    base_macros
    economy
    graphic
    graphic_surface
    logic
    wui_mapview_pixelfunctions
)

wl_library(graphic_terrain_programs
  SRCS
    gl/fields_to_draw.h
    gl/road_program.cc
    gl/road_program.h
    gl/terrain_program.cc
    gl/terrain_program.h
    gl/dither_program.cc
    gl/dither_program.h
  DEPENDS
    base_exceptions
    base_geometry
    base_log
    base_macros
    graphic
    graphic_gl_utils
    graphic_image_io
    graphic_surface
    io_filesystem
    logic
    logic_constants
    logic_widelands_geometry
)


# Font handlers and text

wl_library(graphic_fonthandler_legacy
  SRCS
    font.cc
    font.h
    font_handler.cc
    font_handler.h
    richtext.cc
    richtext.h
    text_parser.cc
    text_parser.h
  USES_ICU
  USES_SDL2_TTF
  DEPENDS
    base_exceptions
    base_geometry
    base_i18n
    base_log
    base_utf8
    graphic
    graphic_align
    graphic_color
    graphic_fonthandler
    graphic_surface
    graphic_text
    graphic_text_constants
    graphic_text_layout
    graphic_wordwrap
    helper
    io_fileread
    io_filesystem
)

wl_library(graphic_fonthandler
  SRCS
    font_handler1.cc
    font_handler1.h
  DEPENDS
    base_macros
    graphic_image_cache
    graphic_text
)

wl_library(graphic_text_constants
  SRCS
    text_constants.h
    text_constants.cc
)

wl_library(graphic_text_layout
  SRCS
    text_layout.cc
    text_layout.h
  DEPENDS
    graphic_align
    graphic_color
    graphic_surface
    graphic_text
    graphic_text_constants
    graphic_fonthandler
)

wl_library(graphic_wordwrap
  SRCS
    wordwrap.cc
    wordwrap.h
  USES_ICU
  USES_SDL2_TTF
  DEPENDS
    base_geometry
    base_log
    graphic
    graphic_align
    graphic_color
    graphic_fonthandler
    graphic_text
    graphic_text_constants
    graphic_text_layout
)


# Base library

wl_library(graphic
  SRCS
    animation.cc
    animation.h
    default_resolution.h
    diranimations.h
    graphic.cc
    graphic.h
    rendertarget.cc
    rendertarget.h
  USES_SDL2
  DEPENDS
    base_exceptions
    base_geometry
    base_i18n
    base_log
    base_macros
    build_info
    graphic_align
    graphic_build_texture_atlas
    graphic_color
    graphic_draw_programs
    graphic_gl_utils
    graphic_image_cache
    graphic_image_io
    graphic_playercolor
    graphic_render_queue
    graphic_surface
    graphic_text_layout
    io_filesystem
    io_stream
    logic_constants
    note_sound
    notifications
    scripting_lua_table
    sound
)
