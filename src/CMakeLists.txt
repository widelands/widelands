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

macro(_parse_common_args ARGS)
  set(OPTIONS
    THIRD_PARTY  # Is a third party lib. Less warnings, no codecheck.
    C_LIBRARY # Pure C library. No CXX flags.
    WIN32 # Windows binary/library.
    USES_BOOST_REGEX
    USES_INTL
    USES_OPENGL
    USES_PNG
    USES_SDL
    USES_SDL_GFX
    USES_SDL_IMAGE
    USES_SDL_MIXER
    USES_SDL_NET
    USES_SDL_TTF
    USES_ZLIB
  )
  set(ONE_VALUE_ARG )
  set(MULTI_VALUE_ARGS SRCS DEPENDS)
  cmake_parse_arguments(ARG "${OPTIONS}" "${ONE_VALUE_ARG}" "${MULTI_VALUE_ARGS}"
    ${ARGS}
  )
endmacro(_parse_common_args)

#include the cmake version dependend macro _include_directories
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include (${CMAKE_SOURCE_DIR}/cmake/IncludeDirectoriesOld.cmake)
else (CMAKE_VERSION VERSION_LESS 2.8.11)
  include (${CMAKE_SOURCE_DIR}/cmake/IncludeDirectories.cmake)
endif (CMAKE_VERSION VERSION_LESS 2.8.11)

function(_include_directories TARGET DIR)
  _include_directories_internal(${TARGET} ${DIR} FALSE)
endfunction(_include_directories TARGET DIR)
function(_include_system_directories TARGET DIR)
  _include_directories_internal(${TARGET} ${DIR} TRUE)
endfunction(_include_system_directories TARGET_DIR)

macro(_common_compile_tasks)
  if (NOT ARG_C_LIBRARY)
    set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} ${WL_GENERIC_CXX_FLAGS}")

    # This is needed for header only libraries. While they do not really mean
    # anything for cmake, they are useful to make dependencies explicit.
    set_target_properties(${NAME} PROPERTIES LINKER_LANGUAGE CXX)
  endif()

  set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} ${WL_OPTIMIZE_FLAGS} ${WL_DEBUG_FLAGS}")

  if(ARG_THIRD_PARTY)
    # Disable all warings for third_party.
    set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} -w")
  else()
    foreach(SRC ${ARG_SRCS})
      run_codecheck(${NAME} ${SRC})
    endforeach(SRC)

    set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} ${WL_COMPILE_DIAGNOSTICS}")
  endif()

  set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS ${TARGET_COMPILE_FLAGS})

  if(NOT ARG_THIRD_PARTY)
    # src/ is the base for all of our includes. The binary one is for generated files.
    _include_directories(${NAME} ${CMAKE_SOURCE_DIR}/src)
    _include_directories(${NAME} ${CMAKE_BINARY_DIR}/src)

    # Boost is practically the standard library, so we always add a search path
    # to include it easily. Except for third party.
    _include_system_directories(${NAME} ${Boost_INCLUDE_DIR})
  endif()

  if(ARG_USES_ZLIB)
    _include_system_directories(${NAME} ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(${NAME} ${ZLIB_LIBRARY})
  endif()

  # OpenGL and GLEW are one thing for us. If you use the one, you also use the
  # other.
  if(ARG_USES_OPENGL)
    _include_system_directories(${NAME} ${GLEW_INCLUDE_DIR})
    target_link_libraries(${NAME} ${GLEW_LIBRARY})
    target_link_libraries(${NAME} ${OPENGL_gl_LIBRARY})
    add_definitions(${GLEW_EXTRA_DEFINITIONS})
  endif()

  if(ARG_USES_PNG)
    _include_system_directories(${NAME} ${PNG_INCLUDE_DIR})
    target_link_libraries(${NAME} ${PNG_LIBRARY})
  endif()

  if(ARG_USES_SDL)
    _include_system_directories(${NAME} ${SDL_INCLUDE_DIR})
    target_link_libraries(${NAME} ${SDL_LIBRARY})
  endif()

  if(ARG_USES_SDL_MIXER)
    _include_system_directories(${NAME} ${SDLMIXER_INCLUDE_DIR})
    target_link_libraries(${NAME} ${SDLMIXER_LIBRARY})
  endif()

  if(ARG_USES_SDL_NET)
    _include_system_directories(${NAME} ${SDLNET_INCLUDE_DIR})
    target_link_libraries(${NAME} ${SDLNET_LIBRARY})
  endif()

  if(ARG_USES_SDL_IMAGE)
    _include_system_directories(${NAME} ${SDLIMAGE_INCLUDE_DIR})
    target_link_libraries(${NAME} ${SDLIMAGE_LIBRARY})
  endif()

  if(ARG_USES_SDL_GFX)
    _include_system_directories(${NAME} ${SDLGFX_INCLUDE_DIR})
    target_link_libraries(${NAME} ${SDLGFX_LIBRARY})
  endif()

  if(ARG_USES_SDL_TTF)
    _include_system_directories(${NAME} ${SDLTTF_INCLUDE_DIR})
    target_link_libraries(${NAME} ${SDLTTF_LIBRARY})
  endif()

  if (ARG_USES_INTL)
    # libintl is not used on all systems, so only include it, when we actually
    # found it.
    if (INTL_FOUND)
      _include_system_directories(${NAME} ${INTL_INCLUDE_DIR})
      target_link_libraries(${NAME} ${INTL_LIBRARY})
    endif()
  endif()

  if (ARG_USES_BOOST_REGEX)
    target_link_libraries(${NAME} ${Boost_LIBRARIES})
  endif()

  foreach(DEPENDENCY ${ARG_DEPENDS})
    target_link_libraries(${NAME} ${DEPENDENCY})
  endforeach(DEPENDENCY)
endmacro(_common_compile_tasks)

# See
# http://www.cmake.org/cmake/help/v3.0/module/CMakeParseArguments.html
function(wl_library NAME)
  _parse_common_args("${ARGN}")

  add_library(${NAME}
    STATIC
    EXCLUDE_FROM_ALL
    ${ARG_SRCS}
  )

  _common_compile_tasks()
endfunction()

function(wl_binary NAME)
  _parse_common_args("${ARGN}")

  if (ARG_WIN32)
    add_executable(${NAME}
      WIN32
      ${ARG_SRCS}
    )
  else()
    add_executable(${NAME}
      ${ARG_SRCS}
    )
  endif()

  _common_compile_tasks()
endfunction()

function(wl_test NAME)
  _parse_common_args("${ARGN}")

  add_executable(${NAME} ${ARG_SRCS})

  # If boost unit test library is linked dynamically, BOOST_TEST_DYN_LINK must be defined
  string(REGEX MATCH ".a$" BOOST_STATIC_UNIT_TEST_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  if (NOT BOOST_STATIC_UNIT_TEST_LIB)
    set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} -DBOOST_TEST_DYN_LINK")
  endif()
  target_link_libraries(${NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

  # Tests need to link with SDL library without main.
  set(SDL_LIBRARY_TEMP ${SDL_LIBRARY})
  list(REMOVE_ITEM SDL_LIBRARY_TEMP ${SDLMAIN_LIBRARY})
  target_link_libraries(${NAME} ${SDL_LIBRARY_TEMP})

  _common_compile_tasks()

  add_test(${NAME} ${NAME})
  add_dependencies(wl_tests ${NAME})
endfunction()

# Checks a single 'SRC' file using Codecheck and writes a file named
# codecheck_<shasum of input> if the codecheck did not yield anything. The
# target for the codecheck will be added as a dependency to 'NAME' for debug
# builds, but always for the target 'codecheck', so that make codecheck checks
# all source code.
function(run_codecheck NAME SRC)
  get_filename_component(ABSOLUTE_SRC ${SRC} ABSOLUTE)

  # If the file does not exist, it is probably auto-generated. In that case, it
  # makes no sense to codecheck it.
  if(EXISTS ${ABSOLUTE_SRC})
    string(SHA1 CHECKSUM ${ABSOLUTE_SRC})

    set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${CHECKSUM}")
    add_custom_command(
      OUTPUT
        ${OUTPUT_FILE}
      COMMAND
        ${CMAKE_COMMAND}
        -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}
        -DWL_SOURCE_CHECKER=${CMAKE_SOURCE_DIR}/cmake/codecheck/CodeCheck.py
        -DSRC=${ABSOLUTE_SRC}
        -DOUTPUT_FILE=${OUTPUT_FILE}
        -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/codecheck/CodeCheck.cmake
      DEPENDS ${ABSOLUTE_SRC}
      COMMENT "Checking ${SRC} with CodeCheck"
    )
    add_custom_target(
      see_if_codecheck_needs_to_run_${CHECKSUM}
      DEPENDS ${OUTPUT_FILE}
      COMMENT ""
    )

    add_dependencies(codecheck see_if_codecheck_needs_to_run_${CHECKSUM})

    if(CMAKE_BUILD_TYPE STREQUAL Debug)
      add_dependencies(${NAME} see_if_codecheck_needs_to_run_${CHECKSUM})
    endif(CMAKE_BUILD_TYPE STREQUAL Debug)
  endif(EXISTS ${ABSOLUTE_SRC})
endfunction(run_codecheck)

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
    USES_SDL
    DEPENDS
      widelands_ball_of_mud
      build_info
  )
else()
  wl_binary(widelands
    SRCS
      main.cc
    USES_SDL
    DEPENDS
      base_exceptions
      widelands_ball_of_mud
      build_info
  )
endif()

add_subdirectory(ai)
add_subdirectory(base)
add_subdirectory(economy)
add_subdirectory(graphic)
add_subdirectory(io)
add_subdirectory(logic)
add_subdirectory(map_io)
add_subdirectory(network)
add_subdirectory(profile)
add_subdirectory(scripting)
add_subdirectory(sound)
add_subdirectory(third_party)
add_subdirectory(ui_basic)
add_subdirectory(wui)

# TODO: Ideally widelands_ball_of_mud shouldn't exist, everything should be in a
# library.
wl_library(widelands_ball_of_mud
  SRCS
    campvis.cc
    campvis.h
    chat.cc
    chat.h
    compile_diagnostics.h
    computer_player.cc
    computer_player.h
    constants.h
    container_iterate.h
    cookie_priority_queue.h
    economy/cmd_call_economy_balance.cc
    economy/cmd_call_economy_balance.h
    economy/economy.cc
    economy/economy.h
    economy/economy_data_packet.cc
    economy/economy_data_packet.h
    economy/flag.cc
    economy/flag.h
    economy/fleet.cc
    economy/fleet.h
    economy/idleworkersupply.cc
    economy/idleworkersupply.h
    economy/iroute.h
    economy/itransport_cost_calculator.h
    economy/portdock.cc
    economy/portdock.h
    economy/request.cc
    economy/request.h
    economy/road.cc
    economy/road.h
    economy/route.cc
    economy/route.h
    economy/routeastar.cc
    economy/routeastar.h
    economy/router.cc
    economy/router.h
    economy/routing_node.h
    economy/shippingitem.cc
    economy/shippingitem.h
    economy/supply.h
    economy/supply_list.cc
    economy/supply_list.h
    economy/transfer.cc
    economy/transfer.h
    economy/ware_instance.cc
    economy/ware_instance.h
    economy/warehousesupply.h
    economy/wares_queue.cc
    economy/wares_queue.h
    editor/editorinteractive.cc
    editor/editorinteractive.h
    editor/tools/editor_action_args.h
    editor/tools/editor_decrease_height_tool.cc
    editor/tools/editor_decrease_height_tool.h
    editor/tools/editor_decrease_resources_tool.cc
    editor/tools/editor_decrease_resources_tool.h
    editor/tools/editor_delete_bob_tool.cc
    editor/tools/editor_delete_bob_tool.h
    editor/tools/editor_delete_immovable_tool.cc
    editor/tools/editor_delete_immovable_tool.h
    editor/tools/editor_draw_tool.cc
    editor/tools/editor_draw_tool.h
    editor/tools/editor_history.cc
    editor/tools/editor_history.h
    editor/tools/editor_increase_height_tool.cc
    editor/tools/editor_increase_height_tool.h
    editor/tools/editor_increase_resources_tool.cc
    editor/tools/editor_increase_resources_tool.h
    editor/tools/editor_info_tool.cc
    editor/tools/editor_info_tool.h
    editor/tools/editor_make_infrastructure_tool.cc
    editor/tools/editor_make_infrastructure_tool.h
    editor/tools/editor_noise_height_tool.cc
    editor/tools/editor_noise_height_tool.h
    editor/tools/editor_place_bob_tool.cc
    editor/tools/editor_place_bob_tool.h
    editor/tools/editor_place_immovable_tool.cc
    editor/tools/editor_place_immovable_tool.h
    editor/tools/editor_set_height_tool.cc
    editor/tools/editor_set_height_tool.h
    editor/tools/editor_set_origin_tool.cc
    editor/tools/editor_set_origin_tool.h
    editor/tools/editor_set_port_space_tool.cc
    editor/tools/editor_set_port_space_tool.h
    editor/tools/editor_set_resources_tool.cc
    editor/tools/editor_set_resources_tool.h
    editor/tools/editor_set_starting_pos_tool.cc
    editor/tools/editor_set_starting_pos_tool.h
    editor/tools/editor_set_terrain_tool.cc
    editor/tools/editor_set_terrain_tool.h
    editor/tools/editor_tool.h
    editor/tools/editor_tool_action.h
    editor/tools/multi_select.h
    editor/ui_menus/categorized_item_selection_menu.h
    editor/ui_menus/editor_main_menu.cc
    editor/ui_menus/editor_main_menu.h
    editor/ui_menus/editor_main_menu_load_map.cc
    editor/ui_menus/editor_main_menu_load_map.h
    editor/ui_menus/editor_main_menu_map_options.cc
    editor/ui_menus/editor_main_menu_map_options.h
    editor/ui_menus/editor_main_menu_new_map.cc
    editor/ui_menus/editor_main_menu_new_map.h
    editor/ui_menus/editor_main_menu_random_map.cc
    editor/ui_menus/editor_main_menu_random_map.h
    editor/ui_menus/editor_main_menu_save_map.cc
    editor/ui_menus/editor_main_menu_save_map.h
    editor/ui_menus/editor_main_menu_save_map_make_directory.cc
    editor/ui_menus/editor_main_menu_save_map_make_directory.h
    editor/ui_menus/editor_player_menu.cc
    editor/ui_menus/editor_player_menu.h
    editor/ui_menus/editor_player_menu_allowed_buildings_menu.cc
    editor/ui_menus/editor_player_menu_allowed_buildings_menu.h
    editor/ui_menus/editor_tool_change_height_options_menu.cc
    editor/ui_menus/editor_tool_change_height_options_menu.h
    editor/ui_menus/editor_tool_change_resources_options_menu.cc
    editor/ui_menus/editor_tool_change_resources_options_menu.h
    editor/ui_menus/editor_tool_menu.cc
    editor/ui_menus/editor_tool_menu.h
    editor/ui_menus/editor_tool_noise_height_options_menu.cc
    editor/ui_menus/editor_tool_noise_height_options_menu.h
    editor/ui_menus/editor_tool_options_menu.cc
    editor/ui_menus/editor_tool_options_menu.h
    editor/ui_menus/editor_tool_place_bob_options_menu.cc
    editor/ui_menus/editor_tool_place_bob_options_menu.h
    editor/ui_menus/editor_tool_place_immovable_options_menu.cc
    editor/ui_menus/editor_tool_place_immovable_options_menu.h
    editor/ui_menus/editor_tool_set_terrain_options_menu.cc
    editor/ui_menus/editor_tool_set_terrain_options_menu.h
    editor/ui_menus/editor_toolsize_menu.cc
    editor/ui_menus/editor_toolsize_menu.h
    game_io/game_cmd_queue_data_packet.cc
    game_io/game_cmd_queue_data_packet.h
    game_io/game_data_packet.h
    game_io/game_game_class_data_packet.cc
    game_io/game_game_class_data_packet.h
    game_io/game_interactive_player_data_packet.cc
    game_io/game_interactive_player_data_packet.h
    game_io/game_loader.cc
    game_io/game_loader.h
    game_io/game_map_data_packet.cc
    game_io/game_map_data_packet.h
    game_io/game_player_economies_data_packet.cc
    game_io/game_player_economies_data_packet.h
    game_io/game_player_info_data_packet.cc
    game_io/game_player_info_data_packet.h
    game_io/game_preload_data_packet.cc
    game_io/game_preload_data_packet.h
    game_io/game_saver.cc
    game_io/game_saver.h
    gamecontroller.h
    helper.cc
    helper.h
    interval.h
    machdep.h
    map_generator.cc
    map_generator.h
    md5.cc
    md5.h
    random.cc
    random.h
    ref_cast.h
    replay_game_controller.cc
    replay_game_controller.h
    save_handler.cc
    save_handler.h
    scoped_timer.cc
    scoped_timer.h
    scripting/c_utils.cc
    scripting/c_utils.h
    scripting/eris.h
    scripting/factory.cc
    scripting/factory.h
    scripting/lua_bases.cc
    scripting/lua_bases.h
    scripting/lua_coroutine.cc
    scripting/lua_coroutine.h
    scripting/lua_editor.cc
    scripting/lua_editor.h
    scripting/lua_errors.h
    scripting/lua_game.cc
    scripting/lua_game.h
    scripting/lua_globals.cc
    scripting/lua_globals.h
    scripting/lua_map.cc
    scripting/lua_map.h
    scripting/lua_path.cc
    scripting/lua_path.h
    scripting/lua_root.cc
    scripting/lua_root.h
    scripting/lua_table.cc
    scripting/lua_table.h
    scripting/lua_ui.cc
    scripting/lua_ui.h
    scripting/luna.h
    scripting/luna_impl.cc
    scripting/luna_impl.h
    scripting/persistence.cc
    scripting/persistence.h
    scripting/scripting.cc
    scripting/scripting.h
    single_player_game_controller.cc
    single_player_game_controller.h
    single_player_game_settings_provider.cc
    single_player_game_settings_provider.h
    text_layout.cc
    text_layout.h
    timestring.cc
    timestring.h
    trackptr.h
    ui_fsmenu/base.cc
    ui_fsmenu/base.h
    ui_fsmenu/campaign_select.cc
    ui_fsmenu/campaign_select.h
    ui_fsmenu/editor.cc
    ui_fsmenu/editor.h
    ui_fsmenu/editor_mapselect.cc
    ui_fsmenu/editor_mapselect.h
    ui_fsmenu/fileview.cc
    ui_fsmenu/fileview.h
    ui_fsmenu/internet_lobby.cc
    ui_fsmenu/internet_lobby.h
    ui_fsmenu/intro.cc
    ui_fsmenu/intro.h
    ui_fsmenu/launch_mpg.cc
    ui_fsmenu/launch_mpg.h
    ui_fsmenu/launch_spg.cc
    ui_fsmenu/launch_spg.h
    ui_fsmenu/loadgame.cc
    ui_fsmenu/loadgame.h
    ui_fsmenu/loadreplay.cc
    ui_fsmenu/loadreplay.h
    ui_fsmenu/main.cc
    ui_fsmenu/main.h
    ui_fsmenu/mapselect.cc
    ui_fsmenu/mapselect.h
    ui_fsmenu/multiplayer.cc
    ui_fsmenu/multiplayer.h
    ui_fsmenu/netsetup_lan.cc
    ui_fsmenu/netsetup_lan.h
    ui_fsmenu/options.cc
    ui_fsmenu/options.h
    ui_fsmenu/singleplayer.cc
    ui_fsmenu/singleplayer.h
    upcast.h
    utf8.h
    wlapplication.cc
    wlapplication.h
  USES_SDL
  DEPENDS
    ai
    base_exceptions
    base_geometry
    base_i18n
    base_log
    build_info
    graphic
    graphic_color
    graphic_image
    graphic_surface
    io_dedicated_log
    io_fileread
    io_filesystem
    io_stream
    logic
    logic_notification
    logic_widelands_geometry
    map_io
    map_io_map_loader
    network
    profile
    sound
    third_party_eris
    ui_basic
    wui
)


if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  target_link_libraries(widelands_ball_of_mud ${EXECINFO_LIBRARY})
endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")

if (WIN32)
  target_link_libraries(widelands_ball_of_mud wsock32)
endif (WIN32)

install(TARGETS widelands DESTINATION ${WL_INSTALL_BINDIR} COMPONENT ExecutableFiles)
