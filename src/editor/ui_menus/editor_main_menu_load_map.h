/*
 * Copyright (C) 2002-2004, 2006, 2008-2011 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_LOAD_MAP_H
#define WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_LOAD_MAP_H

#include <string>

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/window.h"
#include "wui/mapdetails.h"
#include "wui/maptable.h"

struct EditorInteractive;

/**
 * Choose a filename and save your brand new created map
*/
struct MainMenuLoadMap : public UI::Window {
	MainMenuLoadMap(EditorInteractive &);

private:
	void clicked_ok();
	bool set_has_selection();
	void entry_selected();
	void toggle_mapnames();
	void fill_table();

	// UI coordinates and spacers
	int32_t const padding_;      // Common padding between panels
	int32_t const butw_, buth_;  // Button dimensions
	int32_t const tablex_, tabley_, tablew_, tableh_;
	int32_t const right_column_x_;

	MapTable table_;
	MapDetails map_details_;

	UI::Button ok_, cancel_;

	const std::string basedir_;
	std::string curdir_;

	bool has_translated_mapname_;
	UI::Checkbox* cb_dont_localize_mapnames_;
	bool showing_mapames_;
	UI::Button* show_mapnames_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_LOAD_MAP_H
