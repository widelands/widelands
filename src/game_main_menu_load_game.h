/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#ifndef GAME_MAIN_MENU_LOAD_GAME_H
#define GAME_MAIN_MENU_LOAD_GAME_H

#include "ui_unique_window.h"

class Interactive_Player;

namespace UI {
template <typename T> struct Button;
struct EditBox;
struct Textarea;
template <typename T> struct Listselect;
};

struct Game_Main_Menu_Load_Game : public UI::UniqueWindow {
	Game_Main_Menu_Load_Game
		(Interactive_Player *, UI::UniqueWindow::Registry *);
	virtual ~Game_Main_Menu_Load_Game();

private:
	void clicked_ok    ();
	void clicked_cancel();
	void selected      (uint32_t);
	void double_clicked(uint32_t);
	void edit_box_changed();

	void fill_list();
	bool load_game(std::string const & filename);

	Interactive_Player * m_parent;
	UI::Textarea * m_name, * m_gametime;
	UI::Listselect<const char *> * m_ls;

	UI::Button<Game_Main_Menu_Load_Game> * m_ok_btn;
	std::string m_basedir;
	std::string m_curdir;
	std::string m_parentdir;
};

#endif
