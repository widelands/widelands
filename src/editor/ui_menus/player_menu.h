/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_PLAYER_MENU_H
#define WL_EDITOR_UI_MENUS_PLAYER_MENU_H

#include <cstring>
#include <string>
#include <vector>

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/editbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;

class EditorPlayerMenu : public UI::UniqueWindow {
public:
	EditorPlayerMenu(EditorInteractive&, UI::UniqueWindow::Registry&);
	~EditorPlayerMenu() override {
	}

private:
	struct PlayerEdit {
		explicit PlayerEdit(UI::EditBox* init_name,
		                    UI::Button* init_position,
		                    UI::Dropdown<std::string>* init_tribe)
		   : name(init_name), position(init_position), tribe(init_tribe) {
		}
		UI::EditBox* name;
		UI::Button* position;
		UI::Dropdown<std::string>* tribe;
	};

	EditorInteractive& eia();

	void name_changed(int32_t);
	void no_of_players_clicked();
	void player_tribe_clicked(uint8_t);
	void set_starting_pos_clicked(uint8_t);

	void layout() override;

	UI::Box box_;
	std::vector<UI::Box*> rows_;
	std::vector<std::unique_ptr<PlayerEdit>> player_edit_;

	UI::Dropdown<uintptr_t> no_of_players_;

	const std::string default_tribe_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_PLAYER_MENU_H
