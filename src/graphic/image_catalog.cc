/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/image_catalog.h"

#include <cassert>
#include <map>
#include <string>

#include "io/filesystem/layered_filesystem.h"

ImageCatalog::ImageCatalog() {
	init();
}

ImageCatalog::~ImageCatalog() {
	// NOCOM(#codereview): this happens when it is deleted, so not necessary.
	entries_.clear();
}

// Register all images here
void ImageCatalog::init()  {
	// NOCOM(#codereview): should be empty to start with. Also, why not do it in the constructor if you call init() from there anyways?
	entries_.clear();

	// general
	insert(Key::kNoValue, "novalue.png");

	// ui_basic
	insert(Key::kActionContinue, "ui_basic/continue.png");
	insert(Key::kActionDifferent, "ui_basic/different.png");
	insert(Key::kActionStop, "ui_basic/stop.png");
	insert(Key::kButton0, "ui_basic/but0.png");
	insert(Key::kButton1, "ui_basic/but1.png");
	insert(Key::kButton2, "ui_basic/but2.png");
	insert(Key::kButton3, "ui_basic/but3.png");
	insert(Key::kButton4, "ui_basic/but4.png");
	insert(Key::kCaret, "ui_basic/caret.png");
	insert(Key::kCheckbox, "ui_basic/checkbox.png");
	insert(Key::kCheckboxChecked, "ui_basic/checkbox_checked.png");
	insert(Key::kCheckboxEmpty, "ui_basic/checkbox_empty.png");
	insert(Key::kCheckboxLight, "ui_basic/checkbox_light.png");
	insert(Key::kCursor, "ui_basic/cursor.png");
	insert(Key::kCursorClick, "ui_basic/cursor_click.png");
	insert(Key::kFilesDirectory, "ui_basic/ls_dir.png");
	insert(Key::kFilesS2Map, "ui_basic/ls_s2map.png");
	insert(Key::kFilesScenario, "ui_basic/ls_wlscenario.png");
	insert(Key::kFilesWLMap, "ui_basic/ls_wlmap.png");
	insert(Key::kHelp, "ui_basic/menu_help.png");
	insert(Key::kListFirst, "ui_basic/list_first_entry.png");
	insert(Key::kListSelected, "ui_basic/list_selected.png");
	insert(Key::kScrollbarBackground, "ui_basic/scrollbar_background.png");
	insert(Key::kScrollbarDown, "ui_basic/scrollbar_down.png");
	insert(Key::kScrollbarLeft, "ui_basic/scrollbar_left.png");
	insert(Key::kScrollbarRight, "ui_basic/scrollbar_right.png");
	insert(Key::kScrollbarUp, "ui_basic/scrollbar_up.png");
	insert(Key::kSelect, "ui_basic/fsel.png");

	// players
	insert(Key::kPlayerFlag1, "players/genstats_enable_plr_01.png");
	insert(Key::kPlayerFlag2, "players/genstats_enable_plr_02.png");
	insert(Key::kPlayerFlag3, "players/genstats_enable_plr_03.png");
	insert(Key::kPlayerFlag4, "players/genstats_enable_plr_04.png");
	insert(Key::kPlayerFlag5, "players/genstats_enable_plr_05.png");
	insert(Key::kPlayerFlag6, "players/genstats_enable_plr_06.png");
	insert(Key::kPlayerFlag7, "players/genstats_enable_plr_07.png");
	insert(Key::kPlayerFlag8, "players/genstats_enable_plr_08.png");
	insert(Key::kPlayerStartingPosBig1, "players/editor_player_01_starting_pos.png");
	insert(Key::kPlayerStartingPosBig2, "players/editor_player_02_starting_pos.png");
	insert(Key::kPlayerStartingPosBig3, "players/editor_player_03_starting_pos.png");
	insert(Key::kPlayerStartingPosBig4, "players/editor_player_04_starting_pos.png");
	insert(Key::kPlayerStartingPosBig5, "players/editor_player_05_starting_pos.png");
	insert(Key::kPlayerStartingPosBig6, "players/editor_player_06_starting_pos.png");
	insert(Key::kPlayerStartingPosBig7, "players/editor_player_07_starting_pos.png");
	insert(Key::kPlayerStartingPosBig8, "players/editor_player_08_starting_pos.png");
	insert(Key::kPlayerStartingPosSmall1, "players/fsel_editor_set_player_01_pos.png");
	insert(Key::kPlayerStartingPosSmall2, "players/fsel_editor_set_player_02_pos.png");
	insert(Key::kPlayerStartingPosSmall3, "players/fsel_editor_set_player_03_pos.png");
	insert(Key::kPlayerStartingPosSmall4, "players/fsel_editor_set_player_04_pos.png");
	insert(Key::kPlayerStartingPosSmall5, "players/fsel_editor_set_player_05_pos.png");
	insert(Key::kPlayerStartingPosSmall6, "players/fsel_editor_set_player_06_pos.png");
	insert(Key::kPlayerStartingPosSmall7, "players/fsel_editor_set_player_07_pos.png");
	insert(Key::kPlayerStartingPosSmall8, "players/fsel_editor_set_player_08_pos.png");

	// ui_fsmenu
	insert(Key::kFullscreen, "ui_fsmenu/ui_fsmenu.jpg");
	insert(Key::kFullscreenChooseMap, "ui_fsmenu/choosemapmenu.jpg");
	insert(Key::kFullscreenDifficulty2, "ui_fsmenu/easy.png");
	insert(Key::kFullscreenDifficulty3, "ui_fsmenu/challenging.png");
	insert(Key::kFullscreenDifficulty4, "ui_fsmenu/hard.png");
	insert(Key::kFullscreenFileWiew, "ui_fsmenu/fileviewmenu.jpg");
	insert(Key::kFullscreenInternet, "ui_fsmenu/internetmenu.jpg");
	insert(Key::kFullscreenLaunchMPG, "ui_fsmenu/launch_mpg_menu.jpg");
	insert(Key::kFullscreenLoadGame, "ui_fsmenu/menu_load_game.png");
	insert(Key::kFullscreenMain, "ui_fsmenu/mainmenu.jpg");
	insert(Key::kFullscreenOptions, "ui_fsmenu/optionsmenu.jpg");
	insert(Key::kFullscreenRandomTribe, "ui_fsmenu/random.png");
	insert(Key::kFullscreenSharedIn, "ui_fsmenu/shared_in.png");

	// loadscreens
	insert(Key::kLoadscreen, "loadscreens/progress.png");
	insert(Key::kLoadscreenEditor, "loadscreens/editor.jpg");
	insert(Key::kLoadscreenSplash, "loadscreens/splash.jpg");
	insert(Key::kLoadscreenTips, "loadscreens/tips_bg.png");

	// ai
	// TODO(GunChleoc): Need to change the network packets to use all of these.
	// At the moment, we still have a hack in multiplayersetupgroup.
	insert(Key::kAiAggressive, "ai/Aggressive.png");
	insert(Key::kAiDefensive, "ai/Defensive.png");
	insert(Key::kAiNone, "ai/None.png");
	insert(Key::kAiNormal, "ai/Normal.png");
	insert(Key::kAiRandom, "ai/Random.png");

	// wui
	insert(Key::kBackgroundButtonFlat, "wui/ware_list_bg.png");
	insert(Key::kBackgroundButtonFlatSelected, "wui/ware_list_bg_selected.png");
	insert(Key::kBackgroundPlot, "wui/plot_area_bg.png");
	insert(Key::kButtonMenuAbort, "wui/menu_abort.png");
	insert(Key::kButtonMenuOK, "wui/menu_okay.png");
	insert(Key::kWindowBackground, "wui/window_background.png");
	insert(Key::kWindowBorderBottom, "wui/window_bottom.png");
	insert(Key::kWindowBorderLeft, "wui/window_left.png");
	insert(Key::kWindowBorderRight, "wui/window_right.png");
	insert(Key::kWindowBorderTop, "wui/window_top.png");

	// menus
	insert(Key::kMenuBuildhelp, "wui/menus/menu_toggle_buildhelp.png");
	insert(Key::kMenuChat, "wui/menus/menu_chat.png");
	insert(Key::kMenuGoto, "wui/menus/menu_goto.png");
	insert(Key::kMenuMessagesNew, "wui/menus/menu_toggle_newmessage_menu.png");
	insert(Key::kMenuMessagesOld, "wui/menus/menu_toggle_oldmessage_menu.png");
	insert(Key::kMenuMinimap, "wui/menus/menu_toggle_minimap.png");
	insert(Key::kMenuObjectives, "wui/menus/menu_objectives.png");
	insert(Key::kMenuOptions, "wui/menus/menu_options_menu.png");
	insert(Key::kMenuOptionsExit, "wui/menus/menu_exit_game.png");
	insert(Key::kMenuOptionsSave, "wui/menus/menu_save_game.png");
	insert(Key::kMenuStatistics, "wui/menus/menu_toggle_menu.png");
	insert(Key::kMenuStatsBuilding, "wui/menus/menu_building_stats.png");
	insert(Key::kMenuStatsGeneral, "wui/menus/menu_general_stats.png");
	insert(Key::kMenuStatsStock, "wui/menus/menu_stock.png");
	insert(Key::kMenuStatsWare, "wui/menus/menu_ware_stats.png");
	insert(Key::kMenuWatch, "wui/menus/menu_watch_follow.png");

	// stats
	insert(Key::kStatsBuildingsLost, "wui/stats/genstats_civil_blds_lost.png");
	insert(Key::kStatsBuildingsNumber, "wui/stats/genstats_nrbuildings.png");
	insert(Key::kStatsCasualties, "wui/stats/genstats_casualties.png");
	insert(Key::kStatsKills, "wui/stats/genstats_kills.png");
	insert(Key::kStatsLandsize, "wui/stats/genstats_landsize.png");
	insert(Key::kStatsMilitarySitesDefeated, "wui/stats/genstats_msites_defeated.png");
	insert(Key::kStatsMilitarySitesLost, "wui/stats/genstats_msites_lost.png");
	insert(Key::kStatsMilitaryStrength, "wui/stats/genstats_militarystrength.png");
	insert(Key::kStatsPoints, "wui/stats/genstats_points.png");
	insert(Key::kStatsProductivity, "wui/stats/genstats_productivity.png");
	insert(Key::kStatsTabWarehouseWares, "wui/stats/menu_tab_wares_warehouse.png");
	insert(Key::kStatsTabWarehouseWorkers, "wui/stats/menu_tab_workers_warehouse.png");
	insert(Key::kStatsTabWaresConsumption, "wui/stats/menu_tab_wares_consumption.png");
	insert(Key::kStatsTabWaresEconomyHealth, "wui/stats/menu_tab_wares_econ_health.png");
	insert(Key::kStatsTabWaresProduction, "wui/stats/menu_tab_wares_production.png");
	insert(Key::kStatsTabWaresStock, "wui/stats/menu_tab_wares_stock.png");
	insert(Key::kStatsTrees, "wui/stats/genstats_trees.png");
	insert(Key::kStatsWaresNumber, "wui/stats/genstats_nrwares.png");
	insert(Key::kStatsWorkersNumber, "wui/stats/genstats_nrworkers.png");

	// minimap
	insert(Key::kMinimapBuildings, "wui/minimap/button_bldns.png");
	insert(Key::kMinimapFlags, "wui/minimap/button_flags.png");
	insert(Key::kMinimapOwner, "wui/minimap/button_owner.png");
	insert(Key::kMinimapRoads, "wui/minimap/button_roads.png");
	insert(Key::kMinimapTerrain, "wui/minimap/button_terrn.png");
	insert(Key::kMinimapZoom, "wui/minimap/button_zoom.png");

	// messages
	insert(Key::kMessageActionArchive, "wui/messages/message_archive.png");
	insert(Key::kMessageActionRestore, "wui/messages/message_restore.png");
	insert(Key::kMessageArchived, "wui/messages/message_archived.png");
	insert(Key::kMessageNew, "wui/messages/message_new.png");
	insert(Key::kMessageRead, "wui/messages/message_read.png");

	// fieldaction
	insert(Key::kFieldAttack, "wui/fieldaction/menu_tab_attack.png");
	insert(Key::kFieldCensus, "wui/fieldaction/menu_show_census.png");
	insert(Key::kFieldDebug, "wui/fieldaction/menu_debug.png");
	insert(Key::kFieldFlagBuild, "wui/fieldaction/menu_build_flag.png");
	insert(Key::kFieldFlagDestroy, "wui/fieldaction/menu_rip_flag.png");
	insert(Key::kFieldGeologist, "wui/fieldaction/menu_geologist.png");
	insert(Key::kFieldRoadBuild, "wui/fieldaction/menu_build_way.png");
	insert(Key::kFieldRoadDestroy, "wui/fieldaction/menu_rem_way.png");
	insert(Key::kFieldStatistics, "wui/fieldaction/menu_show_statistics.png");
	insert(Key::kFieldTabBuildBig, "wui/fieldaction/menu_tab_buildbig.png");
	insert(Key::kFieldTabBuildMedium, "wui/fieldaction/menu_tab_buildmedium.png");
	insert(Key::kFieldTabBuildMine, "wui/fieldaction/menu_tab_buildmine.png");
	insert(Key::kFieldTabBuildPort, "wui/fieldaction/menu_tab_buildport.png");
	insert(Key::kFieldTabBuildRoad, "wui/fieldaction/menu_tab_buildroad.png");
	insert(Key::kFieldTabBuildSmall, "wui/fieldaction/menu_tab_buildsmall.png");
	insert(Key::kFieldTabWatch, "wui/fieldaction/menu_tab_watch.png");
	insert(Key::kFieldWatch, "wui/fieldaction/menu_watch_field.png");

	// buildings
	insert(Key::kBuildingAttack, "wui/buildings/menu_attack.png");
	insert(Key::kBuildingBulldoze, "wui/buildings/menu_bld_bulldoze.png");
	insert(Key::kBuildingDismantle, "wui/buildings/menu_bld_dismantle.png");
	insert(Key::kBuildingMaxFillIndicator, "wui/buildings/max_fill_indicator.png");
	insert(Key::kBuildingPriorityHigh, "wui/buildings/high_priority_button.png");
	insert(Key::kBuildingPriorityLow, "wui/buildings/low_priority_button.png");
	insert(Key::kBuildingPriorityNormal, "wui/buildings/normal_priority_button.png");
	insert(Key::kBuildingSoldierCapacityDecrease, "wui/buildings/menu_down_train.png");
	insert(Key::kBuildingSoldierCapacityIncrease, "wui/buildings/menu_up_train.png");
	insert(Key::kBuildingSoldierDrop, "wui/buildings/menu_drop_soldier.png");
	insert(Key::kBuildingSoldierHeroes, "wui/buildings/prefer_heroes.png");
	insert(Key::kBuildingSoldierRookies, "wui/buildings/prefer_rookies.png");
	insert(Key::kBuildingStockPolicyDontStock, "wui/buildings/stock_policy_dontstock.png");
	insert(Key::kBuildingStockPolicyDontStockButton, "wui/buildings/stock_policy_button_dontstock.png");
	insert(Key::kBuildingStockPolicyNormalButton, "wui/buildings/stock_policy_button_normal.png");
	insert(Key::kBuildingStockPolicyPrefer, "wui/buildings/stock_policy_prefer.png");
	insert(Key::kBuildingStockPolicyPreferButton, "wui/buildings/stock_policy_button_prefer.png");
	insert(Key::kBuildingStockPolicyRemove, "wui/buildings/stock_policy_remove.png");
	insert(Key::kBuildingStockPolicyRemoveButton, "wui/buildings/stock_policy_button_remove.png");
	insert(Key::kBuildingTabDockWares, "wui/buildings/menu_tab_wares_dock.png");
	insert(Key::kBuildingTabDockWorkers, "wui/buildings/menu_tab_workers_dock.png");
	insert(Key::kBuildingTabMilitary, "wui/buildings/menu_tab_military.png");
	insert(Key::kBuildingTabWarehouseWares, "wui/buildings/menu_tab_wares.png");
	insert(Key::kBuildingTabWarehouseWorkers, "wui/buildings/menu_tab_workers.png");
	insert(Key::kBuildingTabWares, "wui/buildings/menu_tab_wares.png");
	insert(Key::kBuildingTabWorkers, "wui/buildings/menu_list_workers.png");
	insert(Key::kDockExpeditionCancel, "wui/buildings/cancel_expedition.png");
	insert(Key::kDockExpeditionStart, "wui/buildings/start_expedition.png");

	// overlays
	insert(Key::kOverlaysFlag, "wui/overlays/set_flag.png");
	insert(Key::kOverlaysMapSpot, "wui/overlays/map_spot.png");
	insert(Key::kOverlaysPlotBig, "wui/overlays/big.png");
	insert(Key::kOverlaysPlotMedium, "wui/overlays/medium.png");
	insert(Key::kOverlaysPlotMine, "wui/overlays/mine.png");
	insert(Key::kOverlaysPlotPort, "wui/overlays/port.png");
	insert(Key::kOverlaysPlotSmall, "wui/overlays/small.png");
	insert(Key::kOverlaysRoadbuildingAscending, "wui/overlays/roadb_yellow.png");
	insert(Key::kOverlaysRoadbuildingDecending, "wui/overlays/roadb_yellowdown.png");
	insert(Key::kOverlaysRoadbuildingLevel, "wui/overlays/roadb_green.png");
	insert(Key::kOverlaysRoadbuildingSteepAscending, "wui/overlays/roadb_red.png");
	insert(Key::kOverlaysRoadbuildingSteepDecending, "wui/overlays/roadb_reddown.png");
	insert(Key::kOverlaysWorkarea1, "wui/overlays/workarea1.png");
	insert(Key::kOverlaysWorkarea12, "wui/overlays/workarea12.png");
	insert(Key::kOverlaysWorkarea123, "wui/overlays/workarea123.png");
	insert(Key::kOverlaysWorkarea2, "wui/overlays/workarea2.png");
	insert(Key::kOverlaysWorkarea23, "wui/overlays/workarea23.png");
	insert(Key::kOverlaysWorkarea3, "wui/overlays/workarea3.png");

	// ship
	insert(Key::kShipDestination, "wui/ship/menu_ship_destination.png");
	insert(Key::kShipExpeditionCancel, "wui/ship/menu_ship_cancel_expedition.png");
	insert(Key::kShipExploreClockwise, "wui/ship/ship_explore_island_cw.png");
	insert(Key::kShipExploreCounterclockwise, "wui/ship/ship_explore_island_ccw.png");
	insert(Key::kShipGoto, "wui/ship/menu_ship_goto.png");
	insert(Key::kShipScoutEast, "wui/ship/ship_scout_e.png");
	insert(Key::kShipScoutNorthEast, "wui/ship/ship_scout_ne.png");
	insert(Key::kShipScoutNorthWest, "wui/ship/ship_scout_nw.png");
	insert(Key::kShipScoutSouthEast, "wui/ship/ship_scout_se.png");
	insert(Key::kShipScoutSouthWest, "wui/ship/ship_scout_sw.png");
	insert(Key::kShipScoutWest, "wui/ship/ship_scout_w.png");
	insert(Key::kShipSink, "wui/ship/menu_ship_sink.png");

	// editor
	insert(Key::kEditorMenuPlayer, "wui/editor/editor_menu_player_menu.png");
	insert(Key::kEditorMenuToolBob, "wui/editor/editor_menu_tool_place_bob.png");
	insert(Key::kEditorMenuToolHeight, "wui/editor/editor_menu_tool_change_height.png");
	insert(Key::kEditorMenuToolImmovable, "wui/editor/editor_menu_tool_place_immovable.png");
	insert(Key::kEditorMenuToolNoiseHeight, "wui/editor/editor_menu_tool_noise_height.png");
	insert(Key::kEditorMenuToolPortSpace, "wui/editor/editor_menu_tool_set_port_space.png");
	insert(Key::kEditorMenuToolResources, "wui/editor/editor_menu_tool_change_resources.png");
	insert(Key::kEditorMenuTools, "wui/editor/editor_menu_toggle_tool_menu.png");
	insert(Key::kEditorMenuToolSize, "wui/editor/editor_menu_set_toolsize_menu.png");
	insert(Key::kEditorMenuToolTerrain, "wui/editor/editor_menu_tool_set_terrain.png");
	insert(Key::kEditorRedo, "wui/editor/editor_redo.png");
	insert(Key::kEditorTerrainDead, "wui/editor/terrain_dead.png");
	insert(Key::kEditorTerrainDry, "wui/editor/terrain_dry.png");
	insert(Key::kEditorTerrainGreen, "wui/editor/terrain_green.png");
	insert(Key::kEditorTerrainMountain, "wui/editor/terrain_mountain.png");
	insert(Key::kEditorTerrainUnpassable, "wui/editor/terrain_unpassable.png");
	insert(Key::kEditorTerrainWater, "wui/editor/terrain_water.png");
	insert(Key::kEditorToolBob, "wui/editor/fsel_editor_place_bob.png");
	insert(Key::kEditorToolDelete, "wui/editor/fsel_editor_delete.png");
	insert(Key::kEditorToolHeight, "wui/editor/fsel_editor_set_height.png");
	insert(Key::kEditorToolHeightDecrease, "wui/editor/fsel_editor_decrease_height.png");
	insert(Key::kEditorToolHeightIncrease, "wui/editor/fsel_editor_increase_height.png");
	insert(Key::kEditorToolImmovable, "wui/editor/fsel_editor_place_immovable.png");
	insert(Key::kEditorToolInfo, "wui/editor/fsel_editor_info.png");
	insert(Key::kEditorToolNoiseHeight, "wui/editor/fsel_editor_noise_height.png");
	insert(Key::kEditorToolPortSpaceSet, "wui/editor/fsel_editor_set_port_space.png");
	insert(Key::kEditorToolPortSpaceSetSet, "wui/editor/fsel_editor_unset_port_space.png");
	insert(Key::kEditorToolResourcesDecrease, "wui/editor/fsel_editor_decrease_resources.png");
	insert(Key::kEditorToolResourcesDelete, "wui/editor/fsel_editor_delete.png");
	insert(Key::kEditorToolResourcesIncrease, "wui/editor/fsel_editor_increase_resources.png");
	insert(Key::kEditorToolResourcesSet, "wui/editor/fsel_editor_set_resources.png");
	insert(Key::kEditorUndo, "wui/editor/editor_undo.png");

	// logos
	insert(Key::kLogoEditor16, "logos/WL-Editor-16.png");
	insert(Key::kLogoEditor32, "logos/WL-Editor-32.png");
	insert(Key::kLogoEditor64, "logos/WL-Editor-64.png");
	insert(Key::kLogoEditor128, "logos/WL-Editor-128.png");
	insert(Key::kLogoWidelands16, "logos/wl-ico-16.png");
	insert(Key::kLogoWidelands32, "logos/wl-ico-32.png");
	insert(Key::kLogoWidelands48, "logos/wl-ico-48.png");
	insert(Key::kLogoWidelands64, "logos/wl-ico-64.png");
	insert(Key::kLogoWidelands128, "logos/wl-ico-128.png");
	insert(Key::kLogoWidelandsLogo, "logos/wl-logo-64.png");
}

void ImageCatalog::insert(Key key, const std::string& filename) {
	const std::string path = kBaseDir + filename;
	assert(!has_key(key));
	assert(g_fs->file_exists(path));
	entries_.emplace(key, path);
}

const std::string& ImageCatalog::filepath(Key key) const {
	assert(has_key(key));
	return entries_.at(key);
}

bool ImageCatalog::has_key(Key key) const {
	return entries_.count(key) == 1;
}
