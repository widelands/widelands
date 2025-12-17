/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_UI_EDITOR_PLAYER_MENU_H
#define WL_UI_EDITOR_PLAYER_MENU_H

#include <memory>

#include "ui/editor/tool_options_menu.h"
#include "ui/basic/box.h"
#include "ui/basic/button.h"
#include "ui/basic/dropdown.h"
#include "ui/basic/textinput.h"
#include "ui/basic/unique_window.h"

class EditorInteractive;

class EditorPlayerMenu : public EditorToolOptionsMenu {
public:
	EditorPlayerMenu(EditorInteractive&,
	                 EditorSetStartingPosTool& tool,
	                 UI::UniqueWindow::Registry&);

private:
	// Container with UI elements to set a player slot's properties
	struct PlayerEditRow {
		explicit PlayerEditRow(UI::Box* init_box,
		                       UI::EditBox* init_name,
		                       UI::Button* init_position,
		                       UI::Dropdown<std::string>* init_tribe)
		   : box(init_box), name(init_name), position(init_position), tribe(init_tribe) {
		}
		UI::Box* box;
		UI::EditBox* name;
		UI::Button* position;
		UI::Dropdown<std::string>* tribe;
	};

	EditorInteractive& eia() const;

	/// Update player name on map for the given row's player
	void name_changed(size_t row);
	/// Update number of player slots available on the map
	void no_of_players_clicked();
	/// Select tribe for the given row's player
	void player_tribe_clicked(size_t row);
	/// Select starting position for the given row's player
	void set_starting_pos_clicked(size_t row);

	/// Resize window according to number of rows
	void layout() override;

	UI::Box box_;
	UI::Dropdown<uintptr_t> no_of_players_;
	std::vector<std::unique_ptr<PlayerEditRow>> rows_;
};

#endif  // end of include guard: WL_UI_EDITOR_PLAYER_MENU_H
