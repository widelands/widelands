/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef EDITOR_PLAYER_MENU_H
#define EDITOR_PLAYER_MENU_H

#include "constants.h"

#include "widelands.h"

#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"

#include <string>
#include <cstring>
#include <vector>

struct Editor_Interactive;
namespace UI {
struct Textarea;
struct EditBox;
template <typename T, typename ID> struct Callback_IDButton;
}

struct Editor_Player_Menu : public UI::UniqueWindow {
	Editor_Player_Menu
		(Editor_Interactive &, UI::UniqueWindow::Registry &);
	virtual ~Editor_Player_Menu() {}

private:
	UI::UniqueWindow::Registry m_allow_buildings_menu;
	UI::Textarea * m_nr_of_players_ta;
	UI::EditBox * m_plr_names[MAX_PLAYERS];
	UI::Callback_Button<Editor_Player_Menu>
		m_add_player, m_remove_last_player;
	UI::Callback_IDButton<Editor_Player_Menu, Widelands::Player_Number const>
		//* m_plr_allowed_buildings       [MAX_PLAYERS],
		* m_plr_set_pos_buts            [MAX_PLAYERS],
		* m_plr_set_tribes_buts         [MAX_PLAYERS];
	std::vector<std::string> m_tribes;

	int32_t m_spt_index;
	int32_t m_mis_index;

	int32_t m_posy;

	void name_changed(int32_t);
	void clicked_add_player         ();
	void clicked_remove_last_player ();
	void player_tribe_clicked       (uint8_t);
	void set_starting_pos_clicked   (uint8_t);
	void update();
	void think();
};

#endif
