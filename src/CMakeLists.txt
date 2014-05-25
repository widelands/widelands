configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/build_info.cc.cmake ${CMAKE_CURRENT_BINARY_DIR}/build_info.cc)

IF (WIN32)
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/../utils/win32/msvc/widelands.rc.cmake ${CMAKE_CURRENT_BINARY_DIR}/widelands.rc)
  if (NOT WL_VERSION_MAJOR OR WL_VERSION_MAJOR STREQUAL "")
    set (WL_VERSION_MAJOR 0)
  endif (NOT WL_VERSION_MAJOR OR WL_VERSION_MAJOR STREQUAL "")
  if (NOT WL_VERSION_MINOR OR WL_VERSION_MINOR STREQUAL "")
    set (WL_VERSION_MINOR 15)
  endif (NOT WL_VERSION_MINOR OR WL_VERSION_MINOR STREQUAL "")
  if (NOT WL_VERSION_PATCH OR WL_VERSION_PATCH STREQUAL "")
    set (WL_VERSION_PATCH 1)
  endif (NOT WL_VERSION_PATCH OR WL_VERSION_PATCH STREQUAL "")
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/../utils/win32/msvc/Widelands-msvc.iss.cmake ${CMAKE_CURRENT_BINARY_DIR}/Widelands-msvc.iss)
  IF (MINGW)
	set(WIN32_ICON_O ${CMAKE_CURRENT_BINARY_DIR}/wl_icon.o)
	ADD_CUSTOM_COMMAND( OUTPUT ${WIN32_ICON_O}
		COMMAND ${MINGW_PREFIX}windres
			-o${WIN32_ICON_O}
			-i${CMAKE_CURRENT_BINARY_DIR}/widelands.rc
		)
	ENDIF (MINGW)
endif (WIN32)

set(WL_SRCS_CC
	./ai/ai_help_structs.cc
	./ai/ai_hints.cc
	./ai/defaultai.cc
	./align.cc
	./backtrace.cc
	./build_info.cc.cmake
	./campvis.cc
	./chat.cc
	./computer_player.cc
	./debugconsole.cc
	./economy/cmd_call_economy_balance.cc
	./economy/economy.cc
	./economy/economy_data_packet.cc
	./economy/flag.cc
	./economy/fleet.cc
	./economy/idleworkersupply.cc
	./economy/portdock.cc
	./economy/request.cc
	./economy/road.cc
	./economy/routeastar.cc
	./economy/route.cc
	./economy/router.cc
	./economy/shippingitem.cc
	./economy/supply_list.cc
	./economy/transfer.cc
	./economy/ware_instance.cc
	./economy/wares_queue.cc
	./editor/editorinteractive.cc
	./editor/tools/editor_decrease_height_tool.cc
	./editor/tools/editor_decrease_resources_tool.cc
	./editor/tools/editor_delete_bob_tool.cc
	./editor/tools/editor_delete_immovable_tool.cc
	./editor/tools/editor_draw_tool.cc
	./editor/tools/editor_history.cc
	./editor/tools/editor_increase_height_tool.cc
	./editor/tools/editor_increase_resources_tool.cc
	./editor/tools/editor_info_tool.cc
	./editor/tools/editor_make_infrastructure_tool.cc
	./editor/tools/editor_noise_height_tool.cc
	./editor/tools/editor_place_bob_tool.cc
	./editor/tools/editor_place_immovable_tool.cc
	./editor/tools/editor_set_height_tool.cc
	./editor/tools/editor_set_origin_tool.cc
	./editor/tools/editor_set_port_space_tool.cc
	./editor/tools/editor_set_resources_tool.cc
	./editor/tools/editor_set_starting_pos_tool.cc
	./editor/tools/editor_set_terrain_tool.cc
	./editor/ui_menus/editor_main_menu.cc
	./editor/ui_menus/editor_main_menu_load_map.cc
	./editor/ui_menus/editor_main_menu_map_options.cc
	./editor/ui_menus/editor_main_menu_new_map.cc
	./editor/ui_menus/editor_main_menu_random_map.cc
	./editor/ui_menus/editor_main_menu_save_map.cc
	./editor/ui_menus/editor_main_menu_save_map_make_directory.cc
	./editor/ui_menus/editor_player_menu_allowed_buildings_menu.cc
	./editor/ui_menus/editor_player_menu.cc
	./editor/ui_menus/editor_tool_change_height_options_menu.cc
	./editor/ui_menus/editor_tool_change_resources_options_menu.cc
	./editor/ui_menus/editor_tool_menu.cc
	./editor/ui_menus/editor_tool_noise_height_options_menu.cc
	./editor/ui_menus/editor_tool_options_menu.cc
	./editor/ui_menus/editor_tool_place_bob_options_menu.cc
	./editor/ui_menus/editor_tool_place_immovable_options_menu.cc
	./editor/ui_menus/editor_tool_set_terrain_options_menu.cc
	./editor/ui_menus/editor_toolsize_menu.cc
	./exceptions.cc
	./game_io/game_cmd_queue_data_packet.cc
	./game_io/game_game_class_data_packet.cc
	./game_io/game_interactive_player_data_packet.cc
	./game_io/game_loader.cc
	./game_io/game_map_data_packet.cc
	./game_io/game_player_economies_data_packet.cc
	./game_io/game_player_info_data_packet.cc
	./game_io/game_preload_data_packet.cc
	./game_io/game_saver.cc
	./graphic/animation.cc
	./graphic/colormap.cc
	./graphic/font.cc
	./graphic/font_handler1.cc
	./graphic/font_handler.cc
	./graphic/graphic.cc
	./graphic/image_cache.cc
	./graphic/image_loader_impl.cc
	./graphic/image_transformations.cc
	./graphic/in_memory_image.cc
	./graphic/render/gamerenderer.cc
	./graphic/render/gamerenderer_gl.cc
	./graphic/render/gamerenderer_sdl.cc
	./graphic/render/gl_surface.cc
	./graphic/render/gl_surface_screen.cc
	./graphic/render/gl_surface_texture.cc
	./graphic/render/gl_utils.cc
	./graphic/render/minimaprenderer.cc
	./graphic/render/sdl_helper.cc
	./graphic/render/sdl_surface.cc
	./graphic/rendertarget.cc
	./graphic/render/terrain_sdl.cc
	./graphic/richtext.cc
	./graphic/surface_cache.cc
	./graphic/surface.cc
	./graphic/text/rt_parse.cc
	./graphic/text/rt_render.cc
	./graphic/text/sdl_ttf_font_impl.cc
	./graphic/text/sdl_ttf_font_loader_from_file.cc
	./graphic/text/sdl_ttf_font_loader_from_filesystem.cc
	./graphic/text/textstream.cc
	./graphic/texture.cc
	./graphic/wordwrap.cc
	./helper.cc
	./i18n.cc
	./io/dedicated_log.cc
	./io/fileread.cc
	./io/filesystem/disk_filesystem.cc
	./io/filesystem/filesystem.cc
	./io/filesystem/layered_filesystem.cc
	./io/filesystem/zip_filesystem.cc
	./io/filewrite.cc
	./io/streamread.cc
	./io/streamwrite.cc
	./log.cc
	./logic/battle.cc
	./logic/bob.cc
	./logic/buildcost.cc
	./logic/building.cc
	./logic/carrier.cc
	./logic/checkstep.cc
	./logic/cmd_calculate_statistics.cc
	./logic/cmd_expire_message.cc
	./logic/cmd_incorporate.cc
	./logic/cmd_luacoroutine.cc
	./logic/cmd_luascript.cc
	./logic/cmd_queue.cc
	./logic/constructionsite.cc
	./logic/critter_bob.cc
	./logic/dismantlesite.cc
	./logic/editor_game_base.cc
	./logic/expedition_bootstrap.cc
	./logic/field.cc
	./logic/findbob.cc
	./logic/findimmovable.cc
	./logic/findnode.cc
	./logic/game.cc
	./logic/game_data_error.cc
	./logic/immovable.cc
	./logic/instances.cc
	./logic/mapastar.cc
	./logic/map.cc
	./logic/mapdifferenceregion.cc
	./logic/mapfringeregion.cc
	./logic/maphollowregion.cc
	./logic/map_revision.cc
	./logic/maptriangleregion.cc
	./logic/military_data.cc
	./logic/militarysite.cc
	./logic/partially_finished_building.cc
	./logic/path.cc
	./logic/pathfield.cc
	./logic/player.cc
	./logic/playercommand.cc
	./logic/playersmanager.cc
	./logic/production_program.cc
	./logic/productionsite.cc
	./logic/queue_cmd_factory.cc
	./logic/replay.cc
	./logic/requirements.cc
	./logic/ship.cc
	./logic/soldier.cc
	./logic/trainingsite.cc
	./logic/tribe.cc
	./logic/walkingdir.cc
	./logic/ware_descr.cc
	./logic/warehouse.cc
	./logic/warelist.cc
	./logic/widelands_geometry_io.cc
	./logic/worker.cc
	./logic/worker_descr.cc
	./logic/worker_program.cc
	./logic/world.cc
	./map_generator.cc
	./map_io/widelands_map_allowed_building_types_data_packet.cc
	./map_io/widelands_map_allowed_worker_types_data_packet.cc
	./map_io/widelands_map_buildingdata_data_packet.cc
	./map_io/widelands_map_building_data_packet.cc
	./map_io/widelands_map_elemental_data_packet.cc
	./map_io/widelands_map_exploration_data_packet.cc
	./map_io/widelands_map_extradata_data_packet.cc
	./map_io/widelands_map_flagdata_data_packet.cc
	./map_io/widelands_map_flag_data_packet.cc
	./map_io/widelands_map_heights_data_packet.cc
	./map_io/widelands_map_loader.cc
	./map_io/widelands_map_map_object_loader.cc
	./map_io/widelands_map_map_object_saver.cc
	./map_io/widelands_map_node_ownership_data_packet.cc
	./map_io/widelands_map_objective_data_packet.cc
	./map_io/widelands_map_object_packet.cc
	./map_io/widelands_map_player_names_and_tribes_data_packet.cc
	./map_io/widelands_map_player_position_data_packet.cc
	./map_io/widelands_map_players_messages_data_packet.cc
	./map_io/widelands_map_players_view_data_packet.cc
	./map_io/widelands_map_port_spaces_data_packet.cc
	./map_io/widelands_map_resources_data_packet.cc
	./map_io/widelands_map_roaddata_data_packet.cc
	./map_io/widelands_map_road_data_packet.cc
	./map_io/widelands_map_saver.cc
	./map_io/widelands_map_scripting_data_packet.cc
	./map_io/widelands_map_terrain_data_packet.cc
	./map_io/widelands_map_version_data_packet.cc
	./md5.cc
	./network/internet_gaming.cc
	./network/internet_gaming_messages.cc
	./network/netclient.cc
	./network/nethost.cc
	./network/network.cc
	./network/network_gaming_messages.cc
	./network/network_lan_promotion.cc
	./network/network_player_settings_backend.cc
	./profile/profile.cc
	./random.cc
	./replay_game_controller.cc
	./s2map.cc
	./save_handler.cc
	./scoped_timer.cc
	./scripting/c_utils.cc
	./scripting/factory.cc
	./scripting/lua_bases.cc
	./scripting/lua_coroutine.cc
	./scripting/lua_editor.cc
	./scripting/lua_game.cc
	./scripting/lua_globals.cc
	./scripting/lua_map.cc
	./scripting/lua_root.cc
	./scripting/lua_table.cc
	./scripting/lua_ui.cc
	./scripting/luna_impl.cc
	./scripting/persistence.cc
	./scripting/scripting.cc
	./single_player_game_controller.cc
	./single_player_game_settings_provider.cc
	./sound/fxset.cc
	./sound/songset.cc
	./sound/sound_handler.cc
	./text_layout.cc
	./text_parser.cc
	./timestring.cc
	./ui_basic/box.cc
	./ui_basic/button.cc
	./ui_basic/checkbox.cc
	./ui_basic/editbox.cc
	./ui_basic/helpwindow.cc
	./ui_basic/icon.cc
	./ui_basic/icongrid.cc
	./ui_basic/listselect.cc
	./ui_basic/messagebox.cc
	./ui_basic/multilineeditbox.cc
	./ui_basic/multilinetextarea.cc
	./ui_basic/panel.cc
	./ui_basic/progressbar.cc
	./ui_basic/progresswindow.cc
	./ui_basic/radiobutton.cc
	./ui_basic/scrollbar.cc
	./ui_basic/slider.cc
	./ui_basic/spinbox.cc
	./ui_basic/table.cc
	./ui_basic/tabpanel.cc
	./ui_basic/textarea.cc
	./ui_basic/unique_window.cc
	./ui_basic/window.cc
	./ui_fsmenu/base.cc
	./ui_fsmenu/campaign_select.cc
	./ui_fsmenu/editor.cc
	./ui_fsmenu/editor_mapselect.cc
	./ui_fsmenu/fileview.cc
	./ui_fsmenu/internet_lobby.cc
	./ui_fsmenu/intro.cc
	./ui_fsmenu/launchMPG.cc
	./ui_fsmenu/launchSPG.cc
	./ui_fsmenu/loadgame.cc
	./ui_fsmenu/loadreplay.cc
	./ui_fsmenu/main.cc
	./ui_fsmenu/mapselect.cc
	./ui_fsmenu/multiplayer.cc
	./ui_fsmenu/netsetup_lan.cc
	./ui_fsmenu/options.cc
	./ui_fsmenu/singleplayer.cc
	./unique_window_handler.cc
	./wlapplication.cc
	./wui/actionconfirm.cc
	./wui/attack_box.cc
	./wui/building_statistics_menu.cc
	./wui/building_ui.cc
	./wui/buildingwindow.cc
	./wui/chatoverlay.cc
	./wui/constructionsitewindow.cc
	./wui/dismantlesitewindow.cc
	./wui/encyclopedia_window.cc
	./wui/fieldaction.cc
	./wui/game_chat_menu.cc
	./wui/gamechatpanel.cc
	./wui/game_debug_ui.cc
	./wui/game_main_menu.cc
	./wui/game_main_menu_save_game.cc
	./wui/game_message_menu.cc
	./wui/game_objectives_menu.cc
	./wui/game_options_menu.cc
	./wui/game_options_sound_menu.cc
	./wui/game_summary.cc
	./wui/game_tips.cc
	./wui/general_statistics_menu.cc
	./wui/interactive_base.cc
	./wui/interactive_gamebase.cc
	./wui/interactive_player.cc
	./wui/interactive_spectator.cc
	./wui/itemwaresdisplay.cc
	./wui/login_box.cc
	./wui/mapview.cc
	./wui/mapviewpixelfunctions.cc
	./wui/military_box.cc
	./wui/militarysitewindow.cc
	./wui/minimap.cc
	./wui/multiplayersetupgroup.cc
	./wui/overlay_manager.cc
	./wui/playerdescrgroup.cc
	./wui/plot_area.cc
	./wui/portdockwaresdisplay.cc
	./wui/productionsitewindow.cc
	./wui/quicknavigation.cc
	./wui/shipwindow.cc
	./wui/soldiercapacitycontrol.cc
	./wui/soldierlist.cc
	./wui/stock_menu.cc
	./wui/story_message_box.cc
	./wui/trainingsitewindow.cc
	./wui/transport_draw.cc
	./wui/transport_ui.cc
	./wui/warehousewindow.cc
	./wui/waresdisplay.cc
	./wui/waresqueuedisplay.cc
	./wui/ware_statistics_menu.cc
	./wui/watchwindow.cc
)

set(WL_SRCS_H
./ai/ai_help_structs.cc
./ai/ai_help_structs.h
./ai/ai_hints.cc
./ai/ai_hints.h
./ai/defaultai.h
./align.h
./backtrace.h
./build_info.h
./campvis.h
./chat.cc
./chat.h
./CodeCheck.cmake
./compile_diagnostics.h
./computer_player.h
./config.h.cmake
./constants.h
./container_iterate.h
./cookie_priority_queue.h
./debugconsole.h
./description_maintainer.h
./economy/cmd_call_economy_balance.h
./economy/economy_data_packet.h
./economy/economy.h
./economy/flag.h
./economy/fleet.h
./economy/idleworkersupply.h
./economy/iroute.h
./economy/itransport_cost_calculator.h
./economy/portdock.h
./economy/request.h
./economy/road.h
./economy/routeastar.h
./economy/route.h
./economy/router.h
./economy/routing_node.h
./economy/shippingitem.cc
./economy/shippingitem.h
./economy/supply.h
./economy/supply_list.h
./economy/transfer.h
./economy/warehousesupply.h
./economy/ware_instance.h
./economy/wares_queue.h
./editor/editorinteractive.h
./editor/tools/editor_action_args.h
./editor/tools/editor_decrease_height_tool.cc
./editor/tools/editor_decrease_height_tool.h
./editor/tools/editor_decrease_resources_tool.h
./editor/tools/editor_delete_bob_tool.h
./editor/tools/editor_delete_immovable_tool.h
./editor/tools/editor_draw_tool.h
./editor/tools/editor_history.cc
./editor/tools/editor_history.h
./editor/tools/editor_increase_height_tool.cc
./editor/tools/editor_increase_height_tool.h
./editor/tools/editor_increase_resources_tool.h
./editor/tools/editor_info_tool.h
./editor/tools/editor_make_infrastructure_tool.h
./editor/tools/editor_noise_height_tool.cc
./editor/tools/editor_noise_height_tool.h
./editor/tools/editor_place_bob_tool.h
./editor/tools/editor_place_immovable_tool.h
./editor/tools/editor_set_height_tool.cc
./editor/tools/editor_set_height_tool.h
./editor/tools/editor_set_origin_tool.h
./editor/tools/editor_set_port_space_tool.h
./editor/tools/editor_set_resources_tool.h
./editor/tools/editor_set_starting_pos_tool.h
./editor/tools/editor_set_terrain_tool.h
./editor/tools/editor_tool_action.h
./editor/tools/editor_tool.h
./editor/tools/multi_select.h
./editor/ui_menus/editor_main_menu.h
./editor/ui_menus/editor_main_menu_load_map.h
./editor/ui_menus/editor_main_menu_map_options.h
./editor/ui_menus/editor_main_menu_new_map.h
./editor/ui_menus/editor_main_menu_random_map.h
./editor/ui_menus/editor_main_menu_save_map.h
./editor/ui_menus/editor_main_menu_save_map_make_directory.h
./editor/ui_menus/editor_player_menu_allowed_buildings_menu.h
./editor/ui_menus/editor_player_menu.h
./editor/ui_menus/editor_tool_change_height_options_menu.cc
./editor/ui_menus/editor_tool_change_height_options_menu.h
./editor/ui_menus/editor_tool_change_resources_options_menu.cc
./editor/ui_menus/editor_tool_change_resources_options_menu.h
./editor/ui_menus/editor_tool_menu.h
./editor/ui_menus/editor_tool_noise_height_options_menu.cc
./editor/ui_menus/editor_tool_noise_height_options_menu.h
./editor/ui_menus/editor_tool_options_menu.h
./editor/ui_menus/editor_tool_place_bob_options_menu.h
./editor/ui_menus/editor_tool_place_immovable_options_menu.h
./editor/ui_menus/editor_tool_set_terrain_options_menu.h
./editor/ui_menus/editor_toolsize_menu.h
./gamecontroller.h
./game_io/game_cmd_queue_data_packet.h
./game_io/game_data_packet.h
./game_io/game_game_class_data_packet.h
./game_io/game_interactive_player_data_packet.h
./game_io/game_loader.h
./game_io/game_map_data_packet.h
./game_io/game_player_economies_data_packet.h
./game_io/game_player_info_data_packet.h
./game_io/game_preload_data_packet.h
./game_io/game_saver.h
./gamesettings.h
./graphic
./graphic/animation.cc
./graphic/animation.h
./graphic/colormap.cc
./graphic/colormap.h
./graphic/compositemode.h
./graphic/diranimations.h
./graphic/font.cc
./graphic/font.h
./graphic/font_handler1.cc
./graphic/font_handler1.h
./graphic/font_handler.cc
./graphic/font_handler.h
./graphic/graphic.cc
./graphic/graphic.h
./graphic/image_cache.cc
./graphic/image_cache.h
./graphic/image.h
./graphic/image_loader.h
./graphic/image_loader_impl.cc
./graphic/image_loader_impl.h
./graphic/image_transformations.cc
./graphic/image_transformations.h
./graphic/in_memory_image.cc
./graphic/in_memory_image.h
./graphic/render
./graphic/render/gamerenderer.cc
./graphic/render/gamerenderer_gl.cc
./graphic/render/gamerenderer_gl.h
./graphic/render/gamerenderer.h
./graphic/render/gamerenderer_sdl.cc
./graphic/render/gamerenderer_sdl.h
./graphic/render/gl_surface.cc
./graphic/render/gl_surface.h
./graphic/render/gl_surface_screen.cc
./graphic/render/gl_surface_screen.h
./graphic/render/gl_surface_texture.cc
./graphic/render/gl_surface_texture.h
./graphic/render/gl_utils.cc
./graphic/render/gl_utils.h
./graphic/render/minimaprenderer.cc
./graphic/render/minimaprenderer.h
./graphic/render/sdl_helper.cc
./graphic/render/sdl_helper.h
./graphic/render/sdl_surface.cc
./graphic/render/sdl_surface.h
./graphic/rendertarget.cc
./graphic/rendertarget.h
./graphic/render/terrain_sdl.cc
./graphic/render/terrain_sdl.h
./graphic/richtext.cc
./graphic/richtext.h
./graphic/surface_cache.cc
./graphic/surface_cache.h
./graphic/surface.cc
./graphic/surface.h
./graphic/text
./graphic/text/rt_errors.h
./graphic/text/rt_errors_impl.h
./graphic/text/rt_parse.cc
./graphic/text/rt_parse.h
./graphic/text/rt_render.cc
./graphic/text/rt_render.h
./graphic/text/sdl_ttf_font.h
./graphic/text/sdl_ttf_font_impl.cc
./graphic/text/sdl_ttf_font_impl.h
./graphic/text/sdl_ttf_font_loader_from_file.cc
./graphic/text/sdl_ttf_font_loader_from_filesystem.cc
./graphic/text/textstream.cc
./graphic/text/textstream.h
./graphic/texture.cc
./graphic/texture.h
./graphic/wordwrap.cc
./graphic/wordwrap.h
./helper.cc
./helper.h
./i18n.h
./interval.h
./io/dedicated_log.h
./io/fileread.h
./io/filesystem/disk_filesystem.h
./io/filesystem/filesystem_exceptions.h
./io/filesystem/filesystem.h
./io/filesystem/layered_filesystem.h
./io/filesystem/zip_exceptions.h
./io/filesystem/zip_filesystem.h
./io/filewrite.h
./io/streamread.h
./io/streamwrite.h
./log.h
./logic/attackable.h
./logic/battle.h
./logic/bill_of_materials.h
./logic/bob.h
./logic/buildcost.h
./logic/building.h
./logic/carrier.h
./logic/checkstep.cc
./logic/checkstep.h
./logic/cmd_calculate_statistics.h
./logic/cmd_expire_message.h
./logic/cmd_incorporate.h
./logic/cmd_luacoroutine.h
./logic/cmd_luascript.h
./logic/cmd_queue.h
./logic/constructionsite.h
./logic/critter_bob.h
./logic/critter_bob_program.h
./logic/dismantlesite.h
./logic/editor_game_base.h
./logic/expedition_bootstrap.h
./logic/field.h
./logic/findbob.h
./logic/findimmovable.h
./logic/findnode.h
./logic/game_data_error.h
./logic/game.h
./logic/immovable.h
./logic/immovable_program.h
./logic/instances.h
./logic/mapastar.h
./logic/mapdifferenceregion.h
./logic/mapfringeregion.h
./logic/map.h
./logic/maphollowregion.cc
./logic/maphollowregion.h
./logic/mapregion.h
./logic/map_revision.h
./logic/maptriangleregion.h
./logic/message.h
./logic/message_id.h
./logic/message_queue.h
./logic/military_data.h
./logic/militarysite.h
./logic/nodecaps.h
./logic/notification.h
./logic/objective.h
./logic/partially_finished_building.cc
./logic/partially_finished_building.h
./logic/path.cc
./logic/pathfield.cc
./logic/pathfield.h
./logic/path.h
./logic/player_area.h
./logic/playercommand.h
./logic/player.h
./logic/playersmanager.h
./logic/production_program.h
./logic/productionsite.h
./logic/program_result.h
./logic/queue_cmd_factory.h
./logic/queue_cmd_ids.h
./logic/replay.h
./logic/requirements.h
./logic/roadtype.h
./logic/ship.cc
./logic/ship.h
./logic/soldiercontrol.h
./logic/soldier_counts.h
./logic/soldier.h
./logic/tattribute.h
./logic/trainingsite.h
./logic/tribe.h
./logic/walkingdir.h
./logic/ware_descr.h
./logic/warehouse.cc
./logic/warehouse.h
./logic/warelist.h
./logic/wareworker.h
./logic/widelands_geometry.h
./logic/widelands_geometry_io.h
./logic/widelands.h
./logic/worker_descr.h
./logic/worker.h
./logic/worker_program.h
./logic/worlddata.h
./logic/world.h
./logmessage.h
./machdep.h
./map_generator.h
./map_io/map_loader.h
./map_io/widelands_map_allowed_building_types_data_packet.h
./map_io/widelands_map_allowed_worker_types_data_packet.h
./map_io/widelands_map_buildingdata_data_packet.h
./map_io/widelands_map_building_data_packet.h
./map_io/widelands_map_data_packet.h
./map_io/widelands_map_elemental_data_packet.h
./map_io/widelands_map_exploration_data_packet.h
./map_io/widelands_map_extradata_data_packet.h
./map_io/widelands_map_flagdata_data_packet.h
./map_io/widelands_map_flag_data_packet.h
./map_io/widelands_map_heights_data_packet.cc
./map_io/widelands_map_heights_data_packet.h
./map_io/widelands_map_loader.h
./map_io/widelands_map_map_object_loader.h
./map_io/widelands_map_map_object_saver.h
./map_io/widelands_map_message_saver.h
./map_io/widelands_map_node_ownership_data_packet.cc
./map_io/widelands_map_node_ownership_data_packet.h
./map_io/widelands_map_objective_data_packet.h
./map_io/widelands_map_object_packet.h
./map_io/widelands_map_player_names_and_tribes_data_packet.h
./map_io/widelands_map_player_position_data_packet.h
./map_io/widelands_map_players_messages_data_packet.h
./map_io/widelands_map_players_view_data_packet.h
./map_io/widelands_map_port_spaces_data_packet.h
./map_io/widelands_map_resources_data_packet.h
./map_io/widelands_map_roaddata_data_packet.h
./map_io/widelands_map_road_data_packet.h
./map_io/widelands_map_saver.h
./map_io/widelands_map_scripting_data_packet.h
./map_io/widelands_map_terrain_data_packet.h
./map_io/widelands_map_version_data_packet.h
./md5.h
./network/internet_gaming.h
./network/internet_gaming_messages.h
./network/internet_gaming_protocol.h
./network/netclient.h
./network/nethost.cc
./network/nethost.h
./network/network_gaming_messages.h
./network/network.h
./network/network_lan_promotion.h
./network/network_player_settings_backend.h
./network/network_protocol.h
./network/network_system.h
./parse_map_object_types.h
./point.h
./port.h
./profile/profile.h
./random.h
./rect.h
./ref_cast.h
./replay_game_controller.h
./rgbcolor.h
./s2map.h
./save_handler.cc
./save_handler.h
./scoped_timer.h
./scripting/c_utils.h
./scripting/factory.h
./scripting/lua_bases.h
./scripting/lua_coroutine.h
./scripting/lua_editor.h
./scripting/lua_errors.h
./scripting/lua_game.h
./scripting/lua_globals.h
./scripting/lua_map.h
./scripting/lua_root.h
./scripting/lua_table.h
./scripting/lua_ui.h
./scripting/luna.h
./scripting/luna_impl.h
./scripting/persistence.h
./scripting/scripting.h
./single_player_game_controller.h
./single_player_game_settings_provider.h
./singleton.h
./sound/fxset.h
./sound/songset.h
./sound/sound_handler.cc
./sound/sound_handler.h
./text_layout.h
./text_parser.h
./timestring.h
./trackptr.h
./TribeBasicInfo.h
./ui_basic/box.h
./ui_basic/button.h
./ui_basic/checkbox.cc
./ui_basic/checkbox.h
./ui_basic/editbox.h
./ui_basic/helpwindow.cc
./ui_basic/helpwindow.h
./ui_basic/icongrid.h
./ui_basic/icon.h
./ui_basic/listselect.h
./ui_basic/messagebox.h
./ui_basic/mouse_constants.h
./ui_basic/multilineeditbox.h
./ui_basic/multilinetextarea.h
./ui_basic/panel.h
./ui_basic/progressbar.h
./ui_basic/progresswindow.h
./ui_basic/radiobutton.h
./ui_basic/scrollbar.h
./ui_basic/slider.h
./ui_basic/spinbox.h
./ui_basic/table.h
./ui_basic/tabpanel.h
./ui_basic/textarea.h
./ui_basic/unique_window.h
./ui_basic/window.h
./ui_fsmenu/base.h
./ui_fsmenu/campaign_select.h
./ui_fsmenu/editor.h
./ui_fsmenu/editor_mapselect.h
./ui_fsmenu/fileview.h
./ui_fsmenu/internet_lobby.h
./ui_fsmenu/intro.h
./ui_fsmenu/launchMPG.cc
./ui_fsmenu/launchMPG.h
./ui_fsmenu/launchSPG.cc
./ui_fsmenu/launchSPG.h
./ui_fsmenu/loadgame.h
./ui_fsmenu/loadreplay.h
./ui_fsmenu/main.h
./ui_fsmenu/mapselect.h
./ui_fsmenu/multiplayer.h
./ui_fsmenu/netsetup_lan.h
./ui_fsmenu/options.h
./ui_fsmenu/singleplayer.h
./unique_window_handler.cc
./unique_window_handler.h
./upcast.h
./utf8.h
./vector.h
./vertex.h
./warning.h
./wexception.h
./wlapplication.h
./workarea_info.h
./wui/actionconfirm.h
./wui/attack_box.h
./wui/building_statistics_menu.h
./wui/buildingwindow.h
./wui/chatoverlay.cc
./wui/chatoverlay.h
./wui/encyclopedia_window.h
./wui/fieldaction.h
./wui/game_chat_menu.cc
./wui/game_chat_menu.h
./wui/gamechatpanel.cc
./wui/gamechatpanel.h
./wui/game_debug_ui.h
./wui/game_main_menu.h
./wui/game_main_menu_save_game.h
./wui/game_message_menu.h
./wui/game_objectives_menu.h
./wui/game_options_menu.h
./wui/game_options_sound_menu.h
./wui/game_summary.h
./wui/game_tips.h
./wui/general_statistics_menu.h
./wui/interactive_base.h
./wui/interactive_gamebase.h
./wui/interactive_player.h
./wui/interactive_spectator.h
./wui/itemwaresdisplay.h
./wui/login_box.h
./wui/mapview.h
./wui/mapviewpixelconstants.h
./wui/mapviewpixelfunctions.h
./wui/military_box.h
./wui/minimap.h
./wui/multiplayersetupgroup.h
./wui/overlay_manager.h
./wui/playerdescrgroup.h
./wui/plot_area.h
./wui/portdockwaresdisplay.h
./wui/productionsitewindow.h
./wui/quicknavigation.h
./wui/shipwindow.cc
./wui/soldiercapacitycontrol.h
./wui/soldierlist.h
./wui/stock_menu.h
./wui/story_message_box.h
./wui/warehousewindow.cc
./wui/waresdisplay.h
./wui/waresqueuedisplay.h
./wui/ware_statistics_menu.h
./wui/watchwindow.cc
./wui/watchwindow.h
)

#convention: any directory called "test" contains test sources, including subdirectories
set(WL_CODECHECK_SRCS "")
foreach (sourcefile ${WL_SRCS_CC} ${WL_SRCS_H})
  # exclude test sources from Codecheck
  if (NOT ${sourcefile} MATCHES ".*/test/.*")
    list (APPEND WL_CODECHECK_SRCS ${sourcefile})
  endif ()
endforeach (sourcefile ${WL_SRCS_CC} ${WL_SRCS_H})

IF (DEFINED MSVC)
  add_executable (widelands ${GUI_TYPE} main.cc build_info.cc widelands.rc)
else (DEFINED MSVC)
  if (DEFINED MINGW)
    add_executable (widelands ${GUI_TYPE} main.cc build_info.cc ${WIN32_ICON_O})
  else (DEFINED MINGW)
    add_executable (widelands ${GUI_TYPE} main.cc build_info.cc)
  endif (DEFINED MINGW)
endif (DEFINED MSVC)

add_library (widelands_all STATIC ${WL_SRCS_CC} ${WL_SRCS_H})

set (WL_SOURCES_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set (WL_SOURCES_BINARY_ROOT ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  find_package (PythonInterp REQUIRED)
  set (commands "")
  foreach(sourcefile ${WL_CODECHECK_SRCS})
    string(REPLACE "${CMAKE_SOURCE_DIR}/src/" "" sourcewithoutpath ${sourcefile})
    string(REGEX REPLACE "\\.|\\/" "_" sourcenoslash ${sourcewithoutpath})
    if (WIN32)
      string(REGEX REPLACE "^.*:" "" sourcenoslash ${sourcenoslash})
    endif (WIN32)
    get_filename_component(sourcename ${sourcefile} NAME)
    set(stamp ${sourcename}_)
    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/codecheck_${sourcenoslash}"
      COMMAND ${CMAKE_COMMAND} -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} -DWL_SOURCE_CHECKER=${CMAKE_SOURCE_DIR}/cmake/codecheck/CodeCheck.py -Dsourcefile=${sourcefile} -DCMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} -Dstamp=codecheck_${sourcenoslash} -P ${CMAKE_SOURCE_DIR}/src/CodeCheck.cmake
      DEPENDS ${sourcefile}
      COMMENT "Checking ${sourcewithoutpath} with CodeCheck"
    )
    list(APPEND commands ${CMAKE_CURRENT_BINARY_DIR}/codecheck_${sourcenoslash})
  endforeach(sourcefile ${WL_CODECHECK_SRCS})
  add_custom_target(codecheck DEPENDS ${commands})
  add_dependencies(widelands codecheck)
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

#the subdirectories are only used for test definition
#temporarily disabled the 'old' tests due to incompatibility
if (WL_UNIT_TESTS)
  # Tests need to link with SDL library without main
  set(TEST_EXTRA_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/build_info.cc)
  set(TEST_EXTRA_LIBS ${SDL_LIBRARY} )
  list(REMOVE_ITEM TEST_EXTRA_LIBS  ${SDLMAIN_LIBRARY})

  add_subdirectory(economy)
  add_subdirectory(io)
endif (WL_UNIT_TESTS)

add_subdirectory(scripting)

if (NOT MINIZIP_FOUND)
  add_subdirectory(minizip)
  target_link_libraries(widelands_all wl_minizip)
endif (NOT MINIZIP_FOUND)

target_link_libraries(widelands_all eris)
target_link_libraries(widelands_all ${SDLIMAGE_LIBRARY})
target_link_libraries(widelands_all ${SDLMIXER_LIBRARY})
target_link_libraries(widelands_all ${SDLNET_LIBRARY})
target_link_libraries(widelands_all ${SDLTTF_LIBRARY})
target_link_libraries(widelands_all ${SDLGFX_LIBRARY})
target_link_libraries(widelands_all ${PNG_LIBRARY})
target_link_libraries(widelands_all ${ZLIB_LIBRARY})
target_link_libraries(widelands_all ${OPENGL_gl_LIBRARY})
target_link_libraries(widelands_all ${GLEW_LIBRARY})
target_link_libraries(widelands_all ${Boost_LIBRARIES})

if (MINIZIP_FOUND)
  target_link_libraries(widelands_all ${MINIZIP_LIBRARY})
endif (MINIZIP_FOUND)

if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
  target_link_libraries(widelands_all ${EXECINFO_LIBRARY})
endif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" OR CMAKE_SYSTEM_NAME MATCHES "OpenBSD")

#FIXME: Find out whether this is ever used.
if (DEFINED WL_EXTRA_LINK_LIBRARIES)
  target_link_libraries(widelands_all ${WL_EXTRA_LINK_LIBRARIES})
endif (DEFINED WL_EXTRA_LINK_LIBRARIES)

target_link_libraries(widelands widelands_all)
target_link_libraries(widelands ${SDL_LIBRARY})

if (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  target_link_libraries(widelands_all ${INTL_LIBRARY})
endif (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")

if (WIN32)
  if (DEFINED MSVC)
    target_link_libraries(widelands_all ws2_32)

    #gettext on MSVC using 3rdparty bundle needs iconv library
    find_library(ICONV_LIBRARY iconv libiconv REQUIRED)
    target_link_libraries(widelands_all ${ICONV_LIBRARY})

  else (DEFINED MSVC)
    target_link_libraries(widelands_all wsock32)
  endif (DEFINED MSVC)
endif (WIN32)

install(TARGETS widelands DESTINATION ${WL_INSTALL_BINDIR} COMPONENT ExecutableFiles)
