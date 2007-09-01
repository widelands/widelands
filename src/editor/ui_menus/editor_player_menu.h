/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__EDITOR_PLAYER_MENU_H
#define __S__EDITOR_PLAYER_MENU_H

#include "constants.h"
#include "types.h"

#include "ui_button.h"
#include "ui_unique_window.h"

#include <string>
#include <vector>

class Editor_Interactive;
namespace UI {
struct Textarea;
struct Edit_Box;
template <typename T, typename ID> struct IDButton;
};

struct Editor_Player_Menu : public UI::UniqueWindow {
	Editor_Player_Menu
		(Editor_Interactive &, UI::UniqueWindow::Registry *);
      virtual ~Editor_Player_Menu() {}

private:
      UI::UniqueWindow::Registry m_allow_buildings_menu;
      UI::Textarea *m_nr_of_players_ta;
      UI::Edit_Box* m_plr_names[MAX_PLAYERS];
	UI::Button<Editor_Player_Menu>
		m_add_player, m_remove_last_player;
	UI::IDButton<Editor_Player_Menu, const Player_Number>
		//* m_plr_allowed_buildings       [MAX_PLAYERS],
		//* m_plr_make_infrastructure_buts[MAX_PLAYERS],
		* m_plr_set_pos_buts            [MAX_PLAYERS],
		* m_plr_set_tribes_buts         [MAX_PLAYERS];
      std::vector<std::string> m_tribes;

      int m_spt_index;
      int m_mis_index;

      int m_posy;

	void name_changed(int);
	void clicked_add_player         ();
	void clicked_remove_last_player ();
	void player_tribe_clicked       (const Uint8);
	void set_starting_pos_clicked   (const Uint8);
	//void make_infrastructure_clicked(const Uint8);
	//void allowed_buildings_clicked  (const Uint8);
      void update();
      void think();
};

#endif
