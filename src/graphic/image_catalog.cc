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

// NOCOM write a test
// NOCOM write test
#include "graphic/image_catalog.h"

#include <cassert>
#include <map>
#include <string>

#include "base/log.h" // NOCOM
#include "io/filesystem/layered_filesystem.h"

ImageCatalog::ImageCatalog() {
	init();
}

ImageCatalog::~ImageCatalog() {
	entries_.clear();
}

// Register all images here
void ImageCatalog::init()  {
	entries_.clear();
	// ui_basic
	insert(Keys::kButton0, "ui_basic/but0.png");
	insert(Keys::kButton1, "ui_basic/but1.png");
	insert(Keys::kButton2, "ui_basic/but2.png");
	insert(Keys::kButton3, "ui_basic/but3.png");
	insert(Keys::kButton4, "ui_basic/but4.png");
	insert(Keys::kFilesDirectory, "ui_basic/ls_dir.png");
	insert(Keys::kFilesWLMap, "ui_basic/ls_wlmap.png");
	insert(Keys::kFilesS2Map, "ui_basic/ls_s2map.png");
	insert(Keys::kFilesScenario, "ui_basic/ls_wlscenario.png");
	insert(Keys::kScrollbarUp, "ui_basic/scrollbar_up.png");
	insert(Keys::kScrollbarDown, "ui_basic/scrollbar_down.png");
	insert(Keys::kScrollbarLeft, "ui_basic/scrollbar_left.png");
	insert(Keys::kScrollbarRight, "ui_basic/scrollbar_right.png");
	insert(Keys::kScrollbarBackground, "ui_basic/scrollbar_background.png");
	insert(Keys::kSelect, "ui_basic/fsel.png");
	insert(Keys::kCaret, "ui_basic/caret.png");
	insert(Keys::kCheckbox, "ui_basic/checkbox.png");
	insert(Keys::kCheckboxChecked, "ui_basic/checkbox_checked.png");
	insert(Keys::kCheckboxEmpty, "ui_basic/checkbox_empty.png");
	insert(Keys::kCheckboxLight, "ui_basic/checkbox_light.png");
	insert(Keys::kCursor, "ui_basic/cursor.png");
	insert(Keys::kCursor2, "ui_basic/cursor2.png"); // NOCOM unused?
	insert(Keys::kCursorClick, "ui_basic/cursor_click.png");
	insert(Keys::kListFirst, "ui_basic/list_first_entry.png");
	insert(Keys::kListSecond, "ui_basic/list_second_entry.png"); // NOCOM unused?
	insert(Keys::kListThird, "ui_basic/list_third_entry.png"); // NOCOM unused?
	insert(Keys::kListSelected, "ui_basic/list_selected.png");

	// wui
	insert(Keys::kWindowBackground, "wui/window_background.png");
	insert(Keys::kWindowBorderTop, "wui/window_top.png");
	insert(Keys::kWindowBorderLeft, "wui/window_left.png");
	insert(Keys::kWindowBorderRight, "wui/window_right.png");
	insert(Keys::kWindowBorderBottom, "wui/window_bottom.png");
	insert(Keys::kButtonMenuOK, "wui/menu_okay.png");
	insert(Keys::kButtonMenuAbort, "wui/menu_abort.png");

	// ui_fsmenu
	insert(Keys::kFullscreen, "ui_fsmenu/ui_fsmenu.jpg");
	insert(Keys::kFullscreenChooseMap, "ui_fsmenu/choosemapmenu.jpg");
	insert(Keys::kFullscreenFileWiew, "ui_fsmenu/fileviewmenu.jpg");
	insert(Keys::kFullscreenInternet, "ui_fsmenu/internetmenu.jpg");
	insert(Keys::kFullscreenLaunchMPG, "ui_fsmenu/launch_mpg_menu.jpg");
	insert(Keys::kFullscreenMain, "ui_fsmenu/mainmenu.jpg");
	insert(Keys::kFullscreenOptions, "ui_fsmenu/optionsmenu.jpg");
	insert(Keys::kFullscreenSplash, "ui_fsmenu/splash.jpg");

	// loadscreens
	insert(Keys::kLoadscreen, "loadscreens/progress.jpg");
	insert(Keys::kLoadscreenEditor, "loadscreens/editor.jpg");
	insert(Keys::kLoadscreenTips, "loadscreens/tips_bg.png");

	// editor
	insert(Keys::kSelectEditorDelete, "fsel_editor_delete.png");
	insert(Keys::kSelectEditorHeightDecrease, "fsel_editor_decrease_height.png");
	insert(Keys::kSelectEditorHeightIncrease, "fsel_editor_increase_height.png");
	insert(Keys::kSelectEditorResourcesDecrease, "fsel_editor_decrease_resources.png");
	insert(Keys::kSelectEditorResourcesIncrease, "fsel_editor_increase_resources.png");
	insert(Keys::kSelectEditorResourcesDelete, "fsel_editor_delete.png");
	insert(Keys::kSelectEditorInfo, "fsel_editor_info.png");
	insert(Keys::kSelectEditorNoiseHeight, "fsel_editor_noise_height.png");
	insert(Keys::kSelectEditorPlaceBob, "fsel_editor_place_bob.png");
	insert(Keys::kSelectEditorPlaceImmovable, "fsel_editor_place_immovable.png");
	insert(Keys::kSelectEditorSetHeight, "fsel_editor_set_height.png");
	insert(Keys::kSelectEditorSetPortSpace, "fsel_editor_set_port_space.png");
	insert(Keys::kSelectEditorUnsetPortSpace, "fsel_editor_unset_port_space.png");
	insert(Keys::kSelectEditorSetResources, "fsel_editor_set_resources.png");
/*
	insert(Keys::kEditorTerrainDead, "terrain_dead.png");
	insert(Keys::kEditorTerrainDry, "terrain_dry.png");
	insert(Keys::kEditorTerrainGreen, "terrain_green.png");
	insert(Keys::kEditorTerrainMountain, "terrain_mountain.png");
	insert(Keys::kEditorTerrainUnpassable, "terrain_unpassable.png");
	insert(Keys::kEditorTerrainWater, "terrain_water.png");
	insert(Keys::kEditorRedo, "editor_redo.png");
	insert(Keys::kEditorUndo, "editor_undo.png");
	insert(Keys::kEditorMenuPlayer, "editor_menu_player_menu.png");
	insert(Keys::kEditorMenuToolSize, "editor_menu_set_toolsize_menu.png");
	insert(Keys::kEditorMenuTool, "editor_menu_toggle_tool_menu.png");
	insert(Keys::kEditorMenuToolHeight, "editor_menu_tool_change_height.png");
	insert(Keys::kEditorMenuToolResources, "editor_menu_tool_change_resources.png");
	insert(Keys::kEditorMenuToolNoiseHeight, "editor_menu_tool_noise_height.png");
	insert(Keys::kEditorMenuToolBob, "editor_menu_tool_place_bob.png");
	insert(Keys::kEditorMenuToolImmovable, "editor_menu_tool_place_immovable.png");
	insert(Keys::kEditorMenuToolPortSpace, "editor_menu_tool_set_port_space.png");
	insert(Keys::kEditorMenuToolTerrain, "editor_menu_tool_set_terrain.png");
*/
	// players
	insert(Keys::kPlayerStartingPosSmall1, "players/fsel_editor_set_player_01_pos.png");
	insert(Keys::kPlayerStartingPosSmall2, "players/fsel_editor_set_player_02_pos.png");
	insert(Keys::kPlayerStartingPosSmall3, "players/fsel_editor_set_player_03_pos.png");
	insert(Keys::kPlayerStartingPosSmall4, "players/fsel_editor_set_player_04_pos.png");
	insert(Keys::kPlayerStartingPosSmall5, "players/fsel_editor_set_player_05_pos.png");
	insert(Keys::kPlayerStartingPosSmall6, "players/fsel_editor_set_player_06_pos.png");
	insert(Keys::kPlayerStartingPosSmall7, "players/fsel_editor_set_player_07_pos.png");
	insert(Keys::kPlayerStartingPosSmall8, "players/fsel_editor_set_player_08_pos.png");
	insert(Keys::kPlayerStartingPosBig1, "players/editor_player_01_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig2, "players/editor_player_02_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig3, "players/editor_player_03_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig4, "players/editor_player_04_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig5, "players/editor_player_05_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig6, "players/editor_player_06_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig7, "players/editor_player_07_starting_pos.png");
	insert(Keys::kPlayerStartingPosBig8, "players/editor_player_08_starting_pos.png");
	insert(Keys::kPlayerFlag1, "players/genstats_enable_plr_01.png");
	insert(Keys::kPlayerFlag2, "players/genstats_enable_plr_02.png");
	insert(Keys::kPlayerFlag3, "players/genstats_enable_plr_03.png");
	insert(Keys::kPlayerFlag4, "players/genstats_enable_plr_04.png");
	insert(Keys::kPlayerFlag5, "players/genstats_enable_plr_05.png");
	insert(Keys::kPlayerFlag6, "players/genstats_enable_plr_06.png");
	insert(Keys::kPlayerFlag7, "players/genstats_enable_plr_07.png");
	insert(Keys::kPlayerFlag8, "players/genstats_enable_plr_08.png");

	// logos
	insert(Keys::kLogoEditor16, "logos/WL-Editor-16.png");
	insert(Keys::kLogoEditor32, "logos/WL-Editor-32.png");
	insert(Keys::kLogoEditor64, "logos/WL-Editor-64.png");
	insert(Keys::kLogoEditor128, "logos/WL-Editor-128.png");
	insert(Keys::kLogoWidelands16, "logos/wl-ico-16.png");
	insert(Keys::kLogoWidelands32, "logos/wl-ico-32.png");
	insert(Keys::kLogoWidelands48, "logos/wl-ico-48.png");
	insert(Keys::kLogoWidelands64, "logos/wl-ico-64.png");
	insert(Keys::kLogoWidelands128, "logos/wl-ico-128.png");
	insert(Keys::kLogoWidelandsLogo, "logos/wl-logo-64.png");
}

void ImageCatalog::insert(Keys key, const std::string& filename) {
	const std::string path = kBaseDir + filename;
	assert(g_fs->file_exists(path));
	entries_.emplace(key, path);
}

// NOCOM try to get rid of this.
const std::string& ImageCatalog::filepath(Keys key) const {
	assert(has_key(key));
	return entries_.at(key);
}

bool ImageCatalog::has_key(Keys key) const {
	return entries_.count(key) == 1;
}
