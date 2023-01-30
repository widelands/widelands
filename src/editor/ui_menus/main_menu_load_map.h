/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_LOAD_MAP_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_LOAD_MAP_H

#include "editor/editorinteractive.h"
#include "editor/ui_menus/main_menu_load_or_save_map.h"

/**
 * Choose a filename and save your brand new created map
 */
struct MainMenuLoadMap : public MainMenuLoadOrSaveMap {
	explicit MainMenuLoadMap(EditorInteractive& parent, UI::UniqueWindow::Registry& registry);

protected:
	void clicked_ok() override;
	// Sets the current dir and updates labels.
	void set_current_directory(const std::vector<std::string>& filenames) override;

private:
	void entry_selected();
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_LOAD_MAP_H
