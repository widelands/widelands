/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_LOAD_OR_SAVE_MAP_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_LOAD_OR_SAVE_MAP_H

#include "editor/editorinteractive.h"
#include "logic/filesystem_constants.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/mapdetails.h"
#include "wui/maptable.h"

/**
 * Choose a filename and save your brand new created map
 */
struct MainMenuLoadOrSaveMap : public UI::UniqueWindow {
	MainMenuLoadOrSaveMap(EditorInteractive& parent,
	                      UI::UniqueWindow::Registry& registry,
	                      const std::string& name,
	                      const std::string& title,
	                      bool addons,
	                      bool show_empty_dirs = false,
	                      const std::string& basedir = kMapsDir);

protected:
	virtual void clicked_ok() = 0;
	void toggle_mapnames();
	// Sets the current dir and updates labels.
	virtual void set_current_directory(const std::string& filename) = 0;
	void layout() override;
	void fill_table();

	bool compare_players(uint32_t, uint32_t);
	bool compare_mapnames(uint32_t, uint32_t);
	bool compare_size(uint32_t, uint32_t);

	// Private variables first, because compiler would complain about initialization order otherwise
private:
	// Common padding between panels
	int32_t const padding_;

	// Whether to list empty directories
	bool const show_empty_dirs_;

	// Main vertical container for the UI elements
	UI::Box main_box_;

	// Big flexible panel in the middle for the table and map details
	UI::Box table_and_details_box_, table_box_;

protected:
	// Table of maps and its data
	MapTable table_;
	std::vector<MapData> maps_data_;

	// Side panel with details about the currently selected map
	Widelands::EditorGameBase egbase_;
	UI::Box map_details_box_;
	MapDetails map_details_;

	UI::Dropdown<MapData::DisplayType> display_mode_;

	// UI row below the table that can be filled by subclasses
	UI::Box table_footer_box_;

	// Shows name of current directory
	UI::Textarea directory_info_;

	// Bottom row with OK/Cancel buttons
	UI::Box button_box_;
	UI::Button ok_, cancel_;

	// Settings data
	const std::string basedir_;
	std::string curdir_;
	bool include_addon_maps_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_LOAD_OR_SAVE_MAP_H
