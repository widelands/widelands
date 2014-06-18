configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/build_info.cc.cmake ${CMAKE_CURRENT_BINARY_DIR}/build_info.cc)

# Catchall target to codecheck all files.
add_custom_target(codecheck)

# A target that depends on all unit tests as 'make test' will not rebuild
# tests:
# http://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests
add_custom_target(wl_tests)

macro(_parse_common_args ARGS)
  # TODO(sirver): add OPTIONS for the other external libraries we use
  set(OPTIONS THIRD_PARTY C_LIBRARY USES_BOOST_REGEX USES_ZLIB WIN32)
  set(ONE_VALUE_ARG )
  set(MULTI_VALUE_ARGS SRCS DEPENDS)
  cmake_parse_arguments(ARG "${OPTIONS}" "${ONE_VALUE_ARG}" "${MULTI_VALUE_ARGS}"
    ${ARGS}
  )
endmacro(_parse_common_args)

macro(_common_compile_tasks)
  if (NOT ARG_C_LIBRARY)
    set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} ${WL_GENERIC_CXX_FLAGS}")
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
    target_include_directories(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src)

    # Boost is practically the standard library, so we always add a search path
    # to include it easily. Except for third party.
    target_include_directories(${NAME} SYSTEM INTERFACE ${Boost_INCLUDE_DIR})
  endif()

  if(ARG_USES_ZLIB)
    target_include_directories(${NAME} SYSTEM INTERFACE ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(${NAME} ${ZLIB_LIBRARY})
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

  # TODO(sirver): I do not get any unresolved dependency errors
  # when building a STATIC library, only for SHARED. We should
  # always gets those errors to track down cyclic dependencies.
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

set(WL_SRCS
	ai/ai_help_structs.cc
	ai/ai_hints.cc
	ai/defaultai.cc
	campvis.cc
	chat.cc
	computer_player.cc
	economy/cmd_call_economy_balance.cc
	economy/economy.cc
	economy/economy_data_packet.cc
	economy/flag.cc
	economy/fleet.cc
	economy/idleworkersupply.cc
	economy/portdock.cc
	economy/request.cc
	economy/road.cc
	economy/route.cc
	economy/routeastar.cc
	economy/router.cc
	economy/shippingitem.cc
	economy/supply_list.cc
	economy/transfer.cc
	economy/ware_instance.cc
	economy/wares_queue.cc
	editor/editorinteractive.cc
	editor/tools/editor_decrease_height_tool.cc
	editor/tools/editor_decrease_resources_tool.cc
	editor/tools/editor_delete_bob_tool.cc
	editor/tools/editor_delete_immovable_tool.cc
	editor/tools/editor_draw_tool.cc
	editor/tools/editor_history.cc
	editor/tools/editor_increase_height_tool.cc
	editor/tools/editor_increase_resources_tool.cc
	editor/tools/editor_info_tool.cc
	editor/tools/editor_make_infrastructure_tool.cc
	editor/tools/editor_noise_height_tool.cc
	editor/tools/editor_place_bob_tool.cc
	editor/tools/editor_place_immovable_tool.cc
	editor/tools/editor_set_height_tool.cc
	editor/tools/editor_set_origin_tool.cc
	editor/tools/editor_set_port_space_tool.cc
	editor/tools/editor_set_resources_tool.cc
	editor/tools/editor_set_starting_pos_tool.cc
	editor/tools/editor_set_terrain_tool.cc
	editor/ui_menus/editor_main_menu.cc
	editor/ui_menus/editor_main_menu_load_map.cc
	editor/ui_menus/editor_main_menu_map_options.cc
	editor/ui_menus/editor_main_menu_new_map.cc
	editor/ui_menus/editor_main_menu_random_map.cc
	editor/ui_menus/editor_main_menu_save_map.cc
	editor/ui_menus/editor_main_menu_save_map_make_directory.cc
	editor/ui_menus/editor_player_menu.cc
	editor/ui_menus/editor_player_menu_allowed_buildings_menu.cc
	editor/ui_menus/editor_tool_change_height_options_menu.cc
	editor/ui_menus/editor_tool_change_resources_options_menu.cc
	editor/ui_menus/editor_tool_menu.cc
	editor/ui_menus/editor_tool_noise_height_options_menu.cc
	editor/ui_menus/editor_tool_options_menu.cc
	editor/ui_menus/editor_tool_place_bob_options_menu.cc
	editor/ui_menus/editor_tool_place_immovable_options_menu.cc
	editor/ui_menus/editor_tool_set_terrain_options_menu.cc
	editor/ui_menus/editor_toolsize_menu.cc
	game_io/game_cmd_queue_data_packet.cc
	game_io/game_game_class_data_packet.cc
	game_io/game_interactive_player_data_packet.cc
	game_io/game_loader.cc
	game_io/game_map_data_packet.cc
	game_io/game_player_economies_data_packet.cc
	game_io/game_player_info_data_packet.cc
	game_io/game_preload_data_packet.cc
	game_io/game_saver.cc
	graphic/align.cc
	graphic/animation.cc
	graphic/colormap.cc
	graphic/font.cc
	graphic/font_handler.cc
	graphic/font_handler1.cc
	graphic/graphic.cc
	graphic/image_cache.cc
	graphic/image_loader_impl.cc
	graphic/image_transformations.cc
	graphic/in_memory_image.cc
	graphic/render/gamerenderer.cc
	graphic/render/gamerenderer_gl.cc
	graphic/render/gamerenderer_sdl.cc
	graphic/render/gl_surface.cc
	graphic/render/gl_surface_screen.cc
	graphic/render/gl_surface_texture.cc
	graphic/render/gl_utils.cc
	graphic/render/minimaprenderer.cc
	graphic/render/sdl_helper.cc
	graphic/render/sdl_surface.cc
	graphic/render/terrain_sdl.cc
	graphic/rendertarget.cc
	graphic/richtext.cc
	graphic/surface.cc
	graphic/surface_cache.cc
	graphic/text/rt_parse.cc
	graphic/text/rt_render.cc
	graphic/text/sdl_ttf_font_impl.cc
	graphic/text/sdl_ttf_font_loader_from_file.cc
	graphic/text/sdl_ttf_font_loader_from_filesystem.cc
	graphic/text/textstream.cc
	graphic/text_parser.cc
	graphic/texture.cc
	graphic/wordwrap.cc
	helper.cc
	logic/backtrace.cc
	logic/battle.cc
	logic/bob.cc
	logic/buildcost.cc
	logic/building.cc
	logic/carrier.cc
	logic/checkstep.cc
	logic/cmd_calculate_statistics.cc
	logic/cmd_expire_message.cc
	logic/cmd_incorporate.cc
	logic/cmd_luacoroutine.cc
	logic/cmd_luascript.cc
	logic/cmd_queue.cc
	logic/constructionsite.cc
	logic/critter_bob.cc
	logic/dismantlesite.cc
	logic/editor_game_base.cc
	logic/expedition_bootstrap.cc
	logic/field.cc
	logic/findbob.cc
	logic/findimmovable.cc
	logic/findnode.cc
	logic/game.cc
	logic/game_data_error.cc
	logic/immovable.cc
	logic/instances.cc
	logic/map.cc
	logic/map_revision.cc
	logic/mapastar.cc
	logic/mapdifferenceregion.cc
	logic/mapfringeregion.cc
	logic/maphollowregion.cc
	logic/maptriangleregion.cc
	logic/military_data.cc
	logic/militarysite.cc
	logic/partially_finished_building.cc
	logic/path.cc
	logic/pathfield.cc
	logic/player.cc
	logic/playercommand.cc
	logic/playersmanager.cc
	logic/production_program.cc
	logic/productionsite.cc
	logic/queue_cmd_factory.cc
	logic/replay.cc
	logic/requirements.cc
	logic/ship.cc
	logic/soldier.cc
	logic/trainingsite.cc
	logic/tribe.cc
	logic/walkingdir.cc
	logic/ware_descr.cc
	logic/warehouse.cc
	logic/warelist.cc
	logic/widelands_geometry_io.cc
	logic/worker.cc
	logic/worker_descr.cc
	logic/worker_program.cc
	map_generator.cc
	map_io/one_world_legacy_lookup_table.cc
	map_io/one_world_legacy_lookup_table.h
	map_io/widelands_map_allowed_building_types_data_packet.cc
	map_io/widelands_map_allowed_worker_types_data_packet.cc
	map_io/widelands_map_building_data_packet.cc
	map_io/widelands_map_buildingdata_data_packet.cc
	map_io/widelands_map_elemental_data_packet.cc
	map_io/widelands_map_exploration_data_packet.cc
	map_io/widelands_map_extradata_data_packet.cc
	map_io/widelands_map_flag_data_packet.cc
	map_io/widelands_map_flagdata_data_packet.cc
	map_io/widelands_map_heights_data_packet.cc
	map_io/widelands_map_loader.cc
	map_io/widelands_map_map_object_loader.cc
	map_io/widelands_map_map_object_saver.cc
	map_io/widelands_map_node_ownership_data_packet.cc
	map_io/widelands_map_object_packet.cc
	map_io/widelands_map_objective_data_packet.cc
	map_io/widelands_map_player_names_and_tribes_data_packet.cc
	map_io/widelands_map_player_position_data_packet.cc
	map_io/widelands_map_players_messages_data_packet.cc
	map_io/widelands_map_players_view_data_packet.cc
	map_io/widelands_map_port_spaces_data_packet.cc
	map_io/widelands_map_resources_data_packet.cc
	map_io/widelands_map_road_data_packet.cc
	map_io/widelands_map_roaddata_data_packet.cc
	map_io/widelands_map_saver.cc
	map_io/widelands_map_scripting_data_packet.cc
	map_io/widelands_map_terrain_data_packet.cc
	map_io/widelands_map_version_data_packet.cc
	md5.cc
	network/internet_gaming.cc
	network/internet_gaming_messages.cc
	network/netclient.cc
	network/nethost.cc
	network/network.cc
	network/network_gaming_messages.cc
	network/network_lan_promotion.cc
	network/network_player_settings_backend.cc
	random.cc
	replay_game_controller.cc
	s2map.cc
	save_handler.cc
	scoped_timer.cc
	scripting/c_utils.cc
	scripting/factory.cc
	scripting/lua_bases.cc
	scripting/lua_coroutine.cc
	scripting/lua_editor.cc
	scripting/lua_game.cc
	scripting/lua_globals.cc
	scripting/lua_map.cc
	scripting/lua_path.cc
	scripting/lua_root.cc
	scripting/lua_table.cc
	scripting/lua_ui.cc
	scripting/luna_impl.cc
	scripting/persistence.cc
	scripting/scripting.cc
	single_player_game_controller.cc
	single_player_game_settings_provider.cc
	text_layout.cc
	timestring.cc
	ui_basic/box.cc
	ui_basic/button.cc
	ui_basic/checkbox.cc
	ui_basic/editbox.cc
	ui_basic/helpwindow.cc
	ui_basic/icon.cc
	ui_basic/icongrid.cc
	ui_basic/listselect.cc
	ui_basic/messagebox.cc
	ui_basic/multilineeditbox.cc
	ui_basic/multilinetextarea.cc
	ui_basic/panel.cc
	ui_basic/progressbar.cc
	ui_basic/progresswindow.cc
	ui_basic/radiobutton.cc
	ui_basic/scrollbar.cc
	ui_basic/slider.cc
	ui_basic/spinbox.cc
	ui_basic/table.cc
	ui_basic/tabpanel.cc
	ui_basic/textarea.cc
	ui_basic/unique_window.cc
	ui_basic/window.cc
	ui_fsmenu/base.cc
	ui_fsmenu/campaign_select.cc
	ui_fsmenu/editor.cc
	ui_fsmenu/editor_mapselect.cc
	ui_fsmenu/fileview.cc
	ui_fsmenu/internet_lobby.cc
	ui_fsmenu/intro.cc
	ui_fsmenu/launchMPG.cc
	ui_fsmenu/launchSPG.cc
	ui_fsmenu/loadgame.cc
	ui_fsmenu/loadreplay.cc
	ui_fsmenu/main.cc
	ui_fsmenu/mapselect.cc
	ui_fsmenu/multiplayer.cc
	ui_fsmenu/netsetup_lan.cc
	ui_fsmenu/options.cc
	ui_fsmenu/singleplayer.cc
	wlapplication.cc
	wui/actionconfirm.cc
	wui/attack_box.cc
	wui/building_statistics_menu.cc
	wui/building_ui.cc
	wui/buildingwindow.cc
	wui/chatoverlay.cc
	wui/constructionsitewindow.cc
	wui/debugconsole.cc
	wui/dismantlesitewindow.cc
	wui/encyclopedia_window.cc
	wui/fieldaction.cc
	wui/game_chat_menu.cc
	wui/game_debug_ui.cc
	wui/game_main_menu.cc
	wui/game_main_menu_save_game.cc
	wui/game_message_menu.cc
	wui/game_objectives_menu.cc
	wui/game_options_menu.cc
	wui/game_options_sound_menu.cc
	wui/game_summary.cc
	wui/game_tips.cc
	wui/gamechatpanel.cc
	wui/general_statistics_menu.cc
	wui/interactive_base.cc
	wui/interactive_gamebase.cc
	wui/interactive_player.cc
	wui/interactive_spectator.cc
	wui/itemwaresdisplay.cc
	wui/login_box.cc
	wui/mapview.cc
	wui/mapviewpixelfunctions.cc
	wui/military_box.cc
	wui/militarysitewindow.cc
	wui/minimap.cc
	wui/multiplayersetupgroup.cc
	wui/overlay_manager.cc
	wui/playerdescrgroup.cc
	wui/plot_area.cc
	wui/portdockwaresdisplay.cc
	wui/productionsitewindow.cc
	wui/quicknavigation.cc
	wui/shipwindow.cc
	wui/soldiercapacitycontrol.cc
	wui/soldierlist.cc
	wui/stock_menu.cc
	wui/story_message_box.cc
	wui/trainingsitewindow.cc
	wui/transport_draw.cc
	wui/transport_ui.cc
	wui/unique_window_handler.cc
	wui/ware_statistics_menu.cc
	wui/warehousewindow.cc
	wui/waresdisplay.cc
	wui/waresqueuedisplay.cc
	wui/watchwindow.cc
  TribeBasicInfo.h
  ai/ai_help_structs.h
  ai/ai_hints.h
  ai/defaultai.h
  campvis.h
  chat.h
  compile_diagnostics.h
  computer_player.h
  constants.h
  container_iterate.h
  cookie_priority_queue.h
  economy/cmd_call_economy_balance.h
  economy/economy.h
  economy/economy_data_packet.h
  economy/flag.h
  economy/fleet.h
  economy/idleworkersupply.h
  economy/iroute.h
  economy/itransport_cost_calculator.h
  economy/portdock.h
  economy/request.h
  economy/road.h
  economy/route.h
  economy/routeastar.h
  economy/router.h
  economy/routing_node.h
  economy/shippingitem.h
  economy/supply.h
  economy/supply_list.h
  economy/transfer.h
  economy/ware_instance.h
  economy/warehousesupply.h
  economy/wares_queue.h
  editor/editorinteractive.h
  editor/tools/editor_action_args.h
  editor/tools/editor_decrease_height_tool.h
  editor/tools/editor_decrease_resources_tool.h
  editor/tools/editor_delete_bob_tool.h
  editor/tools/editor_delete_immovable_tool.h
  editor/tools/editor_draw_tool.h
  editor/tools/editor_history.h
  editor/tools/editor_increase_height_tool.h
  editor/tools/editor_increase_resources_tool.h
  editor/tools/editor_info_tool.h
  editor/tools/editor_make_infrastructure_tool.h
  editor/tools/editor_noise_height_tool.h
  editor/tools/editor_place_bob_tool.h
  editor/tools/editor_place_immovable_tool.h
  editor/tools/editor_set_height_tool.h
  editor/tools/editor_set_origin_tool.h
  editor/tools/editor_set_port_space_tool.h
  editor/tools/editor_set_resources_tool.h
  editor/tools/editor_set_starting_pos_tool.h
  editor/tools/editor_set_terrain_tool.h
  editor/tools/editor_tool.h
  editor/tools/editor_tool_action.h
  editor/tools/multi_select.h
  editor/ui_menus/editor_main_menu.h
  editor/ui_menus/editor_main_menu_load_map.h
  editor/ui_menus/editor_main_menu_map_options.h
  editor/ui_menus/editor_main_menu_new_map.h
  editor/ui_menus/editor_main_menu_random_map.h
  editor/ui_menus/editor_main_menu_save_map.h
  editor/ui_menus/editor_main_menu_save_map_make_directory.h
  editor/ui_menus/editor_player_menu.h
  editor/ui_menus/editor_player_menu_allowed_buildings_menu.h
  editor/ui_menus/editor_tool_change_height_options_menu.h
  editor/ui_menus/editor_tool_change_resources_options_menu.h
  editor/ui_menus/editor_tool_menu.h
  editor/ui_menus/editor_tool_noise_height_options_menu.h
  editor/ui_menus/editor_tool_options_menu.h
  editor/ui_menus/editor_tool_place_bob_options_menu.h
  editor/ui_menus/editor_tool_place_immovable_options_menu.h
  editor/ui_menus/editor_tool_set_terrain_options_menu.h
  editor/ui_menus/editor_toolsize_menu.h
  game_io/game_cmd_queue_data_packet.h
  game_io/game_data_packet.h
  game_io/game_game_class_data_packet.h
  game_io/game_interactive_player_data_packet.h
  game_io/game_loader.h
  game_io/game_map_data_packet.h
  game_io/game_player_economies_data_packet.h
  game_io/game_player_info_data_packet.h
  game_io/game_preload_data_packet.h
  game_io/game_saver.h
  gamecontroller.h
  gamesettings.h
  graphic/align.h
  graphic/animation.h
  graphic/colormap.h
  graphic/compositemode.h
  graphic/diranimations.h
  graphic/font.h
  graphic/font_handler.h
  graphic/font_handler1.h
  graphic/graphic.h
  graphic/image.h
  graphic/image_cache.h
  graphic/image_loader.h
  graphic/image_loader_impl.h
  graphic/image_transformations.h
  graphic/in_memory_image.h
  graphic/rect.h
  graphic/render/gamerenderer.h
  graphic/render/gamerenderer_gl.h
  graphic/render/gamerenderer_sdl.h
  graphic/render/gl_surface.h
  graphic/render/gl_surface_screen.h
  graphic/render/gl_surface_texture.h
  graphic/render/gl_utils.h
  graphic/render/minimaprenderer.h
  graphic/render/sdl_helper.h
  graphic/render/sdl_surface.h
  graphic/render/terrain_sdl.h
  graphic/render/vertex.h
  graphic/rendertarget.h
  graphic/richtext.h
  graphic/surface.h
  graphic/surface_cache.h
  graphic/text/rt_errors.h
  graphic/text/rt_errors_impl.h
  graphic/text/rt_parse.h
  graphic/text/rt_render.h
  graphic/text/sdl_ttf_font.h
  graphic/text/sdl_ttf_font_impl.h
  graphic/text/textstream.h
  graphic/text_parser.h
  graphic/texture.h
  graphic/wordwrap.h
  helper.h
  interval.h
  logic/attackable.h
  logic/backtrace.h
  logic/battle.h
  logic/bill_of_materials.h
  logic/bob.h
  logic/buildcost.h
  logic/building.h
  logic/carrier.h
  logic/checkstep.h
  logic/cmd_calculate_statistics.h
  logic/cmd_expire_message.h
  logic/cmd_incorporate.h
  logic/cmd_luacoroutine.h
  logic/cmd_luascript.h
  logic/cmd_queue.h
  logic/constructionsite.h
  logic/critter_bob.h
  logic/critter_bob_program.h
  logic/description_maintainer.h
  logic/dismantlesite.h
  logic/editor_game_base.h
  logic/expedition_bootstrap.h
  logic/field.h
  logic/findbob.h
  logic/findimmovable.h
  logic/findnode.h
  logic/game.h
  logic/game_data_error.h
  logic/immovable.h
  logic/immovable_program.h
  logic/instances.h
  logic/map.h
  logic/map_revision.h
  logic/mapastar.h
  logic/mapdifferenceregion.h
  logic/mapfringeregion.h
  logic/maphollowregion.h
  logic/mapregion.h
  logic/maptriangleregion.h
  logic/message.h
  logic/message_id.h
  logic/message_queue.h
  logic/military_data.h
  logic/militarysite.h
  logic/nodecaps.h
  logic/notification.h
  logic/objective.h
  logic/parse_map_object_types.h
  logic/partially_finished_building.h
  logic/path.h
  logic/pathfield.h
  logic/player.h
  logic/player_area.h
  logic/playercommand.h
  logic/playersmanager.h
  logic/production_program.h
  logic/productionsite.h
  logic/program_result.h
  logic/queue_cmd_factory.h
  logic/queue_cmd_ids.h
  logic/replay.h
  logic/requirements.h
  logic/roadtype.h
  logic/ship.h
  logic/soldier.h
  logic/soldier_counts.h
  logic/soldiercontrol.h
  logic/tattribute.h
  logic/trainingsite.h
  logic/tribe.h
  logic/walkingdir.h
  logic/ware_descr.h
  logic/warehouse.h
  logic/warelist.h
  logic/wareworker.h
  logic/widelands.h
  logic/widelands_geometry.h
  logic/widelands_geometry_io.h
  logic/workarea_info.h
  logic/worker.h
  logic/worker_descr.h
  logic/worker_program.h
  logic/world/editor_category.cc
  logic/world/editor_category.h
  logic/world/map_gen.cc
  logic/world/map_gen.h
  logic/world/resource_description.cc
  logic/world/resource_description.h
  logic/world/terrain_description.cc
  logic/world/terrain_description.h
  logic/world/world.cc
  logic/world/world.h
  machdep.h
  map_generator.h
  map_io/coords_profile.cc
  map_io/coords_profile.h
  map_io/map_loader.h
  map_io/widelands_map_allowed_building_types_data_packet.h
  map_io/widelands_map_allowed_worker_types_data_packet.h
  map_io/widelands_map_building_data_packet.h
  map_io/widelands_map_buildingdata_data_packet.h
  map_io/widelands_map_data_packet.h
  map_io/widelands_map_elemental_data_packet.h
  map_io/widelands_map_exploration_data_packet.h
  map_io/widelands_map_extradata_data_packet.h
  map_io/widelands_map_flag_data_packet.h
  map_io/widelands_map_flagdata_data_packet.h
  map_io/widelands_map_heights_data_packet.h
  map_io/widelands_map_loader.h
  map_io/widelands_map_map_object_loader.h
  map_io/widelands_map_map_object_saver.h
  map_io/widelands_map_message_saver.h
  map_io/widelands_map_node_ownership_data_packet.h
  map_io/widelands_map_object_packet.h
  map_io/widelands_map_objective_data_packet.h
  map_io/widelands_map_player_names_and_tribes_data_packet.h
  map_io/widelands_map_player_position_data_packet.h
  map_io/widelands_map_players_messages_data_packet.h
  map_io/widelands_map_players_view_data_packet.h
  map_io/widelands_map_port_spaces_data_packet.h
  map_io/widelands_map_resources_data_packet.h
  map_io/widelands_map_road_data_packet.h
  map_io/widelands_map_roaddata_data_packet.h
  map_io/widelands_map_saver.h
  map_io/widelands_map_scripting_data_packet.h
  map_io/widelands_map_terrain_data_packet.h
  map_io/widelands_map_version_data_packet.h
  md5.h
  network/internet_gaming.h
  network/internet_gaming_messages.h
  network/internet_gaming_protocol.h
  network/netclient.h
  network/nethost.h
  network/network.h
  network/network_gaming_messages.h
  network/network_lan_promotion.h
  network/network_player_settings_backend.h
  network/network_protocol.h
  network/network_system.h
  point.h
  random.h
  ref_cast.h
  replay_game_controller.h
  rgbcolor.h
  s2map.h
  save_handler.h
  scoped_timer.h
  scripting/c_utils.h
  scripting/factory.h
  scripting/lua_bases.h
  scripting/lua_coroutine.h
  scripting/lua_editor.h
  scripting/lua_errors.h
  scripting/lua_game.h
  scripting/lua_globals.h
  scripting/lua_map.h
  scripting/lua_root.h
  scripting/lua_table.h
  scripting/lua_ui.h
  scripting/luna.h
  scripting/luna_impl.h
  scripting/persistence.h
  scripting/scripting.h
  single_player_game_controller.h
  single_player_game_settings_provider.h
  text_layout.h
  timestring.h
  trackptr.h
  ui_basic/box.h
  ui_basic/button.h
  ui_basic/checkbox.h
  ui_basic/editbox.h
  ui_basic/helpwindow.h
  ui_basic/icon.h
  ui_basic/icongrid.h
  ui_basic/listselect.h
  ui_basic/messagebox.h
  ui_basic/mouse_constants.h
  ui_basic/multilineeditbox.h
  ui_basic/multilinetextarea.h
  ui_basic/panel.h
  ui_basic/progressbar.h
  ui_basic/progresswindow.h
  ui_basic/radiobutton.h
  ui_basic/scrollbar.h
  ui_basic/slider.h
  ui_basic/spinbox.h
  ui_basic/table.h
  ui_basic/tabpanel.h
  ui_basic/textarea.h
  ui_basic/unique_window.h
  ui_basic/window.h
  ui_fsmenu/base.h
  ui_fsmenu/campaign_select.h
  ui_fsmenu/editor.h
  ui_fsmenu/editor_mapselect.h
  ui_fsmenu/fileview.h
  ui_fsmenu/internet_lobby.h
  ui_fsmenu/intro.h
  ui_fsmenu/launchMPG.h
  ui_fsmenu/launchSPG.h
  ui_fsmenu/loadgame.h
  ui_fsmenu/loadreplay.h
  ui_fsmenu/main.h
  ui_fsmenu/mapselect.h
  ui_fsmenu/multiplayer.h
  ui_fsmenu/netsetup_lan.h
  ui_fsmenu/options.h
  ui_fsmenu/singleplayer.h
  upcast.h
  utf8.h
  vector.h
  wlapplication.h
  wui/actionconfirm.h
  wui/attack_box.h
  wui/building_statistics_menu.h
  wui/buildingwindow.h
  wui/chatoverlay.h
  wui/debugconsole.h
  wui/encyclopedia_window.h
  wui/fieldaction.h
  wui/game_chat_menu.h
  wui/game_debug_ui.h
  wui/game_main_menu.h
  wui/game_main_menu_save_game.h
  wui/game_message_menu.h
  wui/game_objectives_menu.h
  wui/game_options_menu.h
  wui/game_options_sound_menu.h
  wui/game_summary.h
  wui/game_tips.h
  wui/gamechatpanel.h
  wui/general_statistics_menu.h
  wui/interactive_base.h
  wui/interactive_gamebase.h
  wui/interactive_player.h
  wui/interactive_spectator.h
  wui/itemwaresdisplay.h
  wui/login_box.h
  wui/logmessage.h
  wui/mapview.h
  wui/mapviewpixelconstants.h
  wui/mapviewpixelfunctions.h
  wui/military_box.h
  wui/minimap.h
  wui/multiplayersetupgroup.h
  wui/overlay_manager.h
  wui/playerdescrgroup.h
  wui/plot_area.h
  wui/portdockwaresdisplay.h
  wui/productionsitewindow.h
  wui/quicknavigation.h
  wui/soldiercapacitycontrol.h
  wui/soldierlist.h
  wui/stock_menu.h
  wui/story_message_box.h
  wui/unique_window_handler.h
  wui/ware_statistics_menu.h
  wui/waresdisplay.h
  wui/waresqueuedisplay.h
  wui/watchwindow.h
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
  wl_binary(
    widelands
    WIN32
    SRCS
      main.cc
      ${WIN32_ICON_O}
    DEPENDS
      widelands_ball_of_mud
      build_info
      # TODO(sirver): USE_SDL
      ${SDL_LIBRARY}
  )
else()
  wl_binary(
    widelands
    SRCS
      main.cc
    DEPENDS
      widelands_ball_of_mud
      build_info
      # TODO(sirver): USE_SDL
      ${SDL_LIBRARY}
  )
endif()

add_subdirectory(base)
add_subdirectory(economy)
add_subdirectory(io)
add_subdirectory(profile)
add_subdirectory(scripting)
add_subdirectory(sound)
add_subdirectory(third_party)

#TODO: It looks like various things have cyclic dependencies
#in all directions. Ideally this core shouldn't exist or at least
#be a limited set of header files needed to boostrap the other
#parts. For now it is absolutely everything to make things build :(
wl_library(widelands_ball_of_mud
  SRCS
    ${WL_SRCS}
  DEPENDS
    base_i18n
    third_party_eris
    io_dedicated_log
    io_filesystem
    profile
    sound
    # TODO(sirver): add USES_* for each of these
    ${SDLIMAGE_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${SDLNET_LIBRARY}
    ${SDLTTF_LIBRARY}
    ${SDLGFX_LIBRARY}
    ${PNG_LIBRARY}
    ${OPENGL_gl_LIBRARY}
    ${GLEW_LIBRARY}
)


if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  target_link_libraries(widelands_ball_of_mud ${EXECINFO_LIBRARY})
endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")

if (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  target_link_libraries(widelands_ball_of_mud ${INTL_LIBRARY})
endif (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")

if (WIN32)
  target_link_libraries(widelands_ball_of_mud wsock32)
endif (WIN32)

install(TARGETS widelands DESTINATION ${WL_INSTALL_BINDIR} COMPONENT ExecutableFiles)
