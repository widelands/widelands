/*
 * Copyright (C) 2002-2004, 2006, 2008, 2010-2011 by the Widelands Development Team
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

#ifndef GAME_MAIN_MENU_SAVE_GAME_H
#define GAME_MAIN_MENU_SAVE_GAME_H

#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

#include "i18n.h"

#include "ref_cast.h"

struct Interactive_GameBase;

struct SaveWarnMessageBox;
struct Game_Main_Menu_Save_Game : public UI::UniqueWindow {
	friend struct SaveWarnMessageBox;
	Game_Main_Menu_Save_Game
		(Interactive_GameBase &, UI::UniqueWindow::Registry & registry);

	void fill_list();
private:
	Interactive_GameBase & igbase();
	void die() {UI::UniqueWindow::die();}
	void selected      (uint32_t);
	void double_clicked(uint32_t);
	void edit_box_changed();
	void ok();
	void delete_clicked();

	bool save_game(std::string);

	UI::Listselect<std::string> m_ls;
	UI::EditBox * m_editbox;
	UI::Textarea m_name_label, m_name, m_gametime_label, m_gametime;
	UI::Button * m_button_ok;
	std::string m_curdir;
	std::string m_parentdir;
	std::string m_filename;
	bool m_overwrite;
};

#endif
