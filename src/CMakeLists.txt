configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/build_info.cc.cmake ${CMAKE_CURRENT_BINARY_DIR}/build_info.cc)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/../cmake/datadirversion.cmake ${CMAKE_CURRENT_SOURCE_DIR}/../data/datadirversion)

# Catchall target to codecheck all files.
add_custom_target(codecheck)
add_custom_target(run_build_deps
  COMMAND
    ${PYTHON_EXECUTABLE}
    ${CMAKE_SOURCE_DIR}/utils/build_deps.py
  COMMENT "Checking CMakeLists.txt files."
)
add_dependencies(codecheck run_build_deps)

# A target that depends on all unit tests as 'make test' will not rebuild
# tests:
# https://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
add_custom_target(wl_tests)

#include the cmake version dependend macro _include_directories
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include (${CMAKE_SOURCE_DIR}/cmake/IncludeDirectoriesOld.cmake)
else (CMAKE_VERSION VERSION_LESS 2.8.11)
  include (${CMAKE_SOURCE_DIR}/cmake/IncludeDirectories.cmake)
endif (CMAKE_VERSION VERSION_LESS 2.8.11)

wl_library(build_info
  SRCS
    build_info.cc
    build_info.h
)

if (WIN32)
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/../utils/win32/widelands.rc.cmake ${CMAKE_CURRENT_BINARY_DIR}/widelands.rc)
	message (STATUS "Configured windows resource file")
	set(WIN32_ICON_O ${CMAKE_CURRENT_BINARY_DIR}/wl_icon.o)
	if(MSVC)
		ADD_CUSTOM_COMMAND(OUTPUT ${WIN32_ICON_O}
			COMMAND rc
				/fo ${WIN32_ICON_O}
				${CMAKE_CURRENT_BINARY_DIR}/widelands.rc
		)
	else()
		ADD_CUSTOM_COMMAND(OUTPUT ${WIN32_ICON_O}
			COMMAND ${MINGW_PREFIX}windres
				-o${WIN32_ICON_O}
				-i${CMAKE_CURRENT_BINARY_DIR}/widelands.rc
		)
	endif()

  wl_binary(widelands
    WIN32
    SRCS
      main.cc
      ${WIN32_ICON_O}
    DEPENDS
      base_exceptions
      widelands_ball_of_mud
      build_info
  )
else()
  wl_binary(widelands
    SRCS
      main.cc
    DEPENDS
      base_exceptions
      widelands_ball_of_mud
      build_info
  )
endif()

if(OPTION_FORCE_EMBEDDED_MINIZIP)
    # unset these two if they are cached
    unset(MINIZIP_FOUND)
    set(MINIZIP_STATIC_LIBRARIES "")
else()
   # If pkg-config is ever needed to find other libraries, move the include outside the if() block
  include(FindPkgConfig)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(MINIZIP minizip)
  endif()
endif()

add_subdirectory(third_party)
add_subdirectory(ai)
add_subdirectory(base)
add_subdirectory(chat)
add_subdirectory(economy)
add_subdirectory(editor)
add_subdirectory(game_io)
add_subdirectory(graphic)
add_subdirectory(io)
add_subdirectory(logic)
add_subdirectory(map_io)
add_subdirectory(network)
add_subdirectory(notifications)
add_subdirectory(scripting)
add_subdirectory(sound)
add_subdirectory(ui_basic)
add_subdirectory(ui_fsmenu)
if (OPTION_BUILD_WEBSITE_TOOLS)
  add_subdirectory(website)
endif (OPTION_BUILD_WEBSITE_TOOLS)
add_subdirectory(wui)

# TODO(unknown): Ideally widelands_ball_of_mud shouldn't exist, everything should be in a
# library.
wl_library(widelands_ball_of_mud
  SRCS
    wlapplication.cc
    wlapplication.h
    wlapplication_messages.cc
    wlapplication_messages.h
  USES_ATOMIC
  USES_SDL2
  USES_SDL2_TTF
  DEPENDS
    base
    base_exceptions
    base_geometry
    base_macros
    base_random
    base_time_string
    build_info
    editor
    graphic
    graphic_fonthandler
    graphic_mouse_cursor
    graphic_text
    io_filesystem
    logic
    logic_addons
    logic_exceptions
    logic_filesystem_constants
    logic_game_controller
    logic_game_settings
    logic_map
    logic_single_player_game_settings
    map_io_map_loader
    network
    sound
    ui_basic
    ui_fsmenu
    widelands_options
    wui
)

wl_library(widelands_options
  SRCS
    wlapplication_options.cc
    wlapplication_options.h
    wlapplication_mousewheel_options.cc
    wlapplication_mousewheel_options.h
  DEPENDS
    base
    base_geometry
    graphic_text_layout
    io_filesystem
    io_profile
    logic_filesystem_constants
  USES_SDL2
)

if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  target_link_libraries(widelands_ball_of_mud ${EXECINFO_LIBRARY})
endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")

if (WIN32)
  target_link_libraries(widelands_ball_of_mud wsock32)
#  set_target_properties(widelands PROPERTIES LINK_FLAGS_RELEASE -s)
endif (WIN32)
