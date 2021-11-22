add_subdirectory(animation)
add_subdirectory(styles)
add_subdirectory(text)

# TODO(sirver): Separate this directory into a base directory and one
# that is Widelands aware (can include logic stuff).

# Align and color

wl_library(graphic_toolbar_imageset
  SRCS
    toolbar_imageset.cc
    toolbar_imageset.h
  DEPENDS
    graphic_image_cache
    graphic_surface
    scripting_lua_table
)

wl_library(graphic_align
  SRCS
    align.h
    align.cc
  DEPENDS
    base_geometry
)

wl_library(graphic_color
  SRCS
    color.h
    color.cc
  DEPENDS
    base_string
  USES_SDL2
)

wl_library(graphic_playercolor
  SRCS
    playercolor.cc
    playercolor.h
  DEPENDS
    base_string
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
    base_log
    base_string
    graphic
    graphic_image_io
    graphic_surface
    graphic_texture_atlas
    io_filesystem
    io_profile
    logic_filesystem_constants
)

wl_library(graphic_image_io
  SRCS
    image_io.h
    image_io.cc
  USES_PNG
  USES_SDL2_IMAGE
  DEPENDS
    base_exceptions
    graphic_surface
    io_fileread
    io_filesystem
    io_stream
)

wl_library(graphic_image_cache
  SRCS
    image_cache.cc
    image_cache.h
  DEPENDS
    base_macros
    base_multithreading
    graphic_image_io
    graphic_surface
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
    base_i18n
    base_log
    base_macros
    graphic_text
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
    base_macros
    base_multithreading
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
    graphic_draw_programs
    graphic_fields_to_draw
    graphic_terrain_programs
    logic_map_objects
    logic_map_objects_description_maintainer
)

wl_library(graphic_game_renderer
  SRCS
    game_renderer.cc
    game_renderer.h
  DEPENDS
    graphic
    graphic_fields_to_draw
    graphic_render_queue
    graphic_surface
    logic
    logic_map_objects
)

wl_library(graphic_minimap_renderer
  SRCS
    minimap_renderer.cc
    minimap_renderer.h
  DEPENDS
    base_geometry
    economy
    graphic_playercolor
    graphic_surface
    logic
    logic_constants
    logic_map
    logic_map_objects
    wui_mapview_pixelfunctions
)

wl_library(graphic_fields_to_draw
  SRCS
    gl/fields_to_draw.cc
    gl/fields_to_draw.h
  DEPENDS
    base_geometry
    base_log
    graphic
    graphic_gl_utils
    logic
    logic_constants
    logic_map_objects
    logic_widelands_geometry
    wui_mapview_pixelfunctions
)

wl_library(graphic_terrain_programs
  SRCS
    gl/grid_program.cc
    gl/grid_program.h
    gl/road_program.cc
    gl/road_program.h
    gl/terrain_program.cc
    gl/terrain_program.h
    gl/dither_program.cc
    gl/dither_program.h
    gl/workarea_program.cc
    gl/workarea_program.h
  USES_ATOMIC
  DEPENDS
    base_exceptions
    base_geometry
    base_macros
    graphic
    graphic_fields_to_draw
    graphic_gl_utils
    graphic_image_io
    graphic_surface
    io_filesystem
    logic
    logic_map_objects
    logic_map_objects_description_maintainer
    wui_mapview_pixelconstants
)


# Font handler and text
wl_library(graphic_fonthandler
  SRCS
    font_handler.cc
    font_handler.h
  DEPENDS
    base_macros
    base_multithreading
    graphic_image_cache
    graphic_text
)

wl_library(graphic_text_layout
  SRCS
    text_layout.cc
    text_layout.h
  DEPENDS
    base_string
    graphic # TODO(Gunchleoc): For text_width safety only - rewrite that function
    graphic_align
    graphic_color
    graphic_surface
    graphic_styles
    graphic_text
    graphic_fonthandler
    graphic_styles
)

wl_library(graphic_wordwrap
  SRCS
    wordwrap.cc
    wordwrap.h
  USES_ICU
  DEPENDS
    base_geometry
    base_string
    graphic
    graphic_align
    graphic_color
    graphic_fonthandler
    graphic_text
    graphic_text_layout
)


# Mouse cursor

wl_library(graphic_mouse_cursor
  SRCS
    mouse_cursor.cc
    mouse_cursor.h
  USES_SDL2
  DEPENDS
    base_geometry
    graphic
    graphic_image_cache
    graphic_image_io
    io_filesystem
)


# Misc high-level helper and convenience functions

wl_library(graphic_functions
  SRCS
    graphic_functions.cc
    graphic_functions.h
  DEPENDS
    graphic
    graphic_fonthandler
    graphic_text_layout
)

# Base library

wl_library(graphic
  SRCS
    default_resolution.h
    graphic.cc
    graphic.h
    note_graphic_resolution_changed.h
    style_manager.cc
    style_manager.h
    rendertarget.cc
    rendertarget.h
    road_segments.cc
    road_segments.h
  USES_SDL2
  DEPENDS
    base_exceptions
    base_geometry
    base_i18n
    base_log
    base_scoped_timer
    base_times
    build_info
    graphic_align
    graphic_animation
    graphic_build_texture_atlas
    graphic_color
    graphic_draw_programs
    graphic_gl_utils
    graphic_image_cache
    graphic_image_io
    graphic_render_queue
    graphic_styles
    graphic_surface
    io_filesystem
    io_stream
    logic_widelands_geometry
    notifications
    scripting_lua_interface
    scripting_lua_table
)

