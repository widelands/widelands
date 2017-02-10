/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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
#include <map>
#include <string>
#include <vector>

#include "graphic/playercolor.h"
#include "logic/widelands.h"
#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;
namespace UI {
struct Textarea;
struct EditBox;
struct Button;
}

class EditorPlayerMenu : public UI::UniqueWindow {
public:
	EditorPlayerMenu(EditorInteractive&, UI::UniqueWindow::Registry&);
	virtual ~EditorPlayerMenu() {
	}

private:
	EditorInteractive& eia();
	UI::UniqueWindow::Registry allow_buildings_menu_;
	UI::Textarea* nr_of_players_ta_;
	UI::EditBox* plr_names_[kMaxPlayers];
	UI::Button add_player_, remove_last_player_;
	UI::Button *plr_make_infrastructure_buts_[kMaxPlayers], *plr_set_pos_buts_[kMaxPlayers],
	   *plr_set_tribes_buts_[kMaxPlayers];

	std::vector<std::string> tribenames_;

	/// List of the tribes currently selected for all players
	std::string selected_tribes_[kMaxPlayers];

	int32_t posy_;

	void name_changed(int32_t);
	void clicked_add_player();
	void clicked_remove_last_player();
	void player_tribe_clicked(uint8_t);
	void set_starting_pos_clicked(uint8_t);
	void update();
	void think() override;
	void make_infrastructure_clicked(uint8_t);
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_PLAYER_MENU_H
