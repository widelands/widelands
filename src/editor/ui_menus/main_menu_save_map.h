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

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_SAVE_MAP_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_SAVE_MAP_H

#include <memory>

#include "editor/editorinteractive.h"
#include "editor/ui_menus/main_menu_load_or_save_map.h"
#include "editor/ui_menus/main_menu_map_options.h"
#include "logic/note_map_options.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/textarea.h"

/**
 * Choose a filename and save your brand new created map
 */
struct MainMenuSaveMap : public MainMenuLoadOrSaveMap {
	explicit MainMenuSaveMap(EditorInteractive& parent,
	                         UI::UniqueWindow::Registry& registry,
	                         UI::UniqueWindow::Registry& map_options_registry);

protected:
	// Sets the current dir and updates labels.
	void set_current_directory(const std::string& filename) override;
	void layout() override;

private:
	EditorInteractive& eia();
	Registry& map_options_registry_;

	void clicked_ok() override;
	void clicked_make_directory();
	void clicked_edit_options();
	void clicked_item();
	void double_clicked_item();
	void edit_box_changed();
	/// Resets the map's filename in the editbox. If mapname didn't change, die().
	void reset_editbox_or_die(const std::string& current_filename);

	void update_map_options();

	bool save_map(std::string, bool);

	UI::Button edit_options_;

	UI::Textarea editbox_label_;
	UI::EditBox editbox_;
	UI::Button make_directory_;

	const std::string illegal_filename_tooltip_;

	std::unique_ptr<Notifications::Subscriber<NoteMapOptions>> subscriber_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_SAVE_MAP_H
