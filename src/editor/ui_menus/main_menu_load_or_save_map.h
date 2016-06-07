/*
 * Copyright (C) 2002-2004, 2006, 2008-2015 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_LOAD_OR_SAVE_MAP_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_LOAD_OR_SAVE_MAP_H

#include <string>

#include "editor/editorinteractive.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"
#include "wui/mapdetails.h"
#include "wui/maptable.h"

/**
 * Choose a filename and save your brand new created map
*/
struct MainMenuLoadOrSaveMap : public UI::Window {
	MainMenuLoadOrSaveMap(EditorInteractive& parent,
	                      int no_of_bottom_rows,
	                      const std::string& name,
	                      const std::string& title,
	                      const std::string& basedir = "maps");

protected:
	virtual void clicked_ok() = 0;
	void toggle_mapnames();
	// Sets the current dir and updates labels.
	virtual void set_current_directory(const std::string& filename) = 0;
	void fill_table();

	bool compare_players(uint32_t, uint32_t);
	bool compare_mapnames(uint32_t, uint32_t);
	bool compare_size(uint32_t, uint32_t);

	// UI coordinates and spacers
	int32_t const padding_;  // Common padding between panels
	int32_t const buth_;     // Button dimensions
	int32_t const tablex_, tabley_, tablew_, tableh_;
	int32_t const right_column_x_;
	int32_t const butw_;  // Button dimensions

	MapTable table_;
	std::vector<MapData> maps_data_;
	MapDetails map_details_;

	UI::Textarea directory_info_;
	UI::Button ok_, cancel_;

	const std::string basedir_;
	std::string curdir_;

	bool has_translated_mapname_;
	UI::Checkbox* cb_dont_localize_mapnames_;
	bool showing_mapames_;
	UI::Button* show_mapnames_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_LOAD_OR_SAVE_MAP_H
