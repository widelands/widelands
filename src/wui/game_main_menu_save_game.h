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

#ifndef GAME_MAIN_MENU_SAVE_GAME_H
#define GAME_MAIN_MENU_SAVE_GAME_H

#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_listselect.h"
#include "ui_messagebox.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

struct Interactive_GameBase;

struct SaveWarnMessageBox;
struct Game_Main_Menu_Save_Game : public UI::UniqueWindow {
	friend struct SaveWarnMessageBox;
	Game_Main_Menu_Save_Game
		(Interactive_GameBase &, UI::UniqueWindow::Registry & registry);

private:
	Interactive_GameBase & igbase();
	void clicked_ok    ();
	void selected      (uint32_t);
	void double_clicked(uint32_t);
	void edit_box_changed();

	void fill_list();
	bool save_game(std::string);

	UI::Listselect<std::string> m_ls;
	UI::EditBox m_editbox;
	UI::Textarea m_name_label, m_name, m_gametime_label, m_gametime;
	UI::Callback_Button<Game_Main_Menu_Save_Game> m_button_ok, m_button_cancel;
	std::string m_curdir;
	std::string m_parentdir;
	std::string m_filename;
	bool m_overwrite;
};

#endif
