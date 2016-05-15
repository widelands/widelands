/*
 * Copyright (C) 2002-2004, 2006, 2008-2009 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_SAVE_MAP_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_SAVE_MAP_H

#include <string>

#include "editor/editorinteractive.h"
#include "editor/ui_menus/main_menu_load_or_save_map.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/textarea.h"

/**
 * Choose a filename and save your brand new created map
*/
struct MainMenuSaveMap : public MainMenuLoadOrSaveMap {
	MainMenuSaveMap(EditorInteractive& parent);

private:
	EditorInteractive & eia();
	void clicked_ok();
	void clicked_make_directory();
	void clicked_edit_options();
	void clicked_item();
	void double_clicked_item();
	void edit_box_changed();

	// Sets the current dir and updates labels.
	void set_current_directory(const std::string& filename);

	bool save_map(std::string, bool);

	UI::Button make_directory_, edit_options_;

	UI::Textarea editbox_label_, directory_info_;
	UI::EditBox* editbox_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_SAVE_MAP_H
