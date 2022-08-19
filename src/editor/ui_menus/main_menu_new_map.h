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

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_NEW_MAP_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_NEW_MAP_H

#include "editor/ui_menus/map_size_box.h"
#include "logic/widelands.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;

/**
 * This is the new map selection menu. It offers
 * the user to choose the new world and a few other
 * things like size, world ....
 */
struct MainMenuNewMap : public UI::UniqueWindow {
	explicit MainMenuNewMap(EditorInteractive&, UI::UniqueWindow::Registry&);
	bool handle_key(bool down, SDL_Keysym code) override;

private:
	EditorInteractive& eia();
	void clicked_create_map();
	void clicked_cancel();
	void fill_list();

	int32_t margin_;
	int32_t box_width_;
	UI::Box box_;
	MapSizeBox map_size_box_;

	// Terrains list
	UI::Listselect<Widelands::DescriptionIndex> list_;

	// Buttons
	UI::Box button_box_;
	UI::Button ok_button_, cancel_button_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_NEW_MAP_H
