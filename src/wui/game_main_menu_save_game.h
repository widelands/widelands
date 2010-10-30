/*
 * Copyright (C) 2002-2004, 2006, 2008, 2010 by the Widelands Development Team
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

	bool save_game(std::string);

	UI::Listselect<std::string> m_ls;
	struct EditBox : public UI::EditBox {
		EditBox
			(Game_Main_Menu_Save_Game & parent,
			 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h)
			:
			UI::EditBox
				(&parent,
				 x, y, w, h,
				 g_gr->get_picture(PicMod_UI, "pics/but1.png"))
		{
			changed.set(&parent, &Game_Main_Menu_Save_Game::edit_box_changed);
		}
		bool handle_key(bool down, SDL_keysym);
	} m_editbox;
	UI::Textarea m_name_label, m_name, m_gametime_label, m_gametime;
	struct Ok : public UI::Button {
		Ok
			(Game_Main_Menu_Save_Game & parent,
			 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h)
			:
			UI::Button
				(&parent, "ok",
				 x, y, w, h,
				 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
				 _("OK"),
				 std::string(),
				 false)
		{}
		void clicked();
	} m_button_ok;
	struct Cancel : public UI::Button {
		Cancel
			(Game_Main_Menu_Save_Game & parent,
			 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h)
			:
			UI::Button
				(&parent, "cancel",
				 x, y, w, h,
				 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
				 _("Cancel"))
		{}
		void clicked() {
			ref_cast<Game_Main_Menu_Save_Game, UI::Panel>(*get_parent()).die();
		}
	} m_button_cancel;
	struct Delete : public UI::Button {
		Delete
			(Game_Main_Menu_Save_Game & parent,
			 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h)
			:
			UI::Button
				(&parent, "delete",
				 x, y, w, h,
				 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
				 _("Delete"))
		{}
		void clicked();
	} m_button_delete;
	std::string m_curdir;
	std::string m_parentdir;
	std::string m_filename;
	bool m_overwrite;
};

#endif
