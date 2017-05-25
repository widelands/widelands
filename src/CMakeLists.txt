configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/build_info.cc.cmake ${CMAKE_CURRENT_BINARY_DIR}/build_info.cc)

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
# http://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
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
	ADD_CUSTOM_COMMAND(OUTPUT ${WIN32_ICON_O}
		COMMAND ${MINGW_PREFIX}windres
			-o${WIN32_ICON_O}
			-i${CMAKE_CURRENT_BINARY_DIR}/widelands.rc
  )
  wl_binary(widelands
    WIN32
    SRCS
      main.cc
      ${WIN32_ICON_O}
    USES_SDL2
    DEPENDS
      base_log
      base_exceptions
      widelands_ball_of_mud
      build_info
  )
else()
  wl_binary(widelands
    SRCS
      main.cc
    USES_SDL2
    DEPENDS
      base_log
      base_exceptions
      widelands_ball_of_mud
      build_info
  )
endif()

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
add_subdirectory(profile)
add_subdirectory(random)
add_subdirectory(scripting)
add_subdirectory(sound)
add_subdirectory(third_party)
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
  USES_SDL2
  USES_SDL2_IMAGE
  USES_SDL2_TTF
  DEPENDS
    base_exceptions
    base_geometry
    base_i18n
    base_log
    base_time_string
    build_info
    editor
    graphic
    graphic_fonthandler
    graphic_fonthandler_legacy
    graphic_text
    graphic_text_constants
    helper
    io_filesystem
    logic
    logic_game_controller
    logic_game_settings
    map_io_map_loader
    network
    profile
    sound
    ui_basic
    ui_fsmenu
    ui_fsmenu_options
    wui
)

# TODO(sirver): Split into libs with useful names.
wl_library(helper
  SRCS
    helper.cc
    helper.h
  USES_SDL2
  DEPENDS
    base_exceptions
)

if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  target_link_libraries(widelands_ball_of_mud ${EXECINFO_LIBRARY})
endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")

if (WIN32)
  target_link_libraries(widelands_ball_of_mud wsock32)
endif (WIN32)
