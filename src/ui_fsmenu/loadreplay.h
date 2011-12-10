/*
 * Copyright (C) 2007-2009 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_LOADREPLAY_H
#define FULLSCREEN_MENU_LOADREPLAY_H

#include "base.h"

#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/textarea.h"


/**
 * Select a replay from a list of replays.
 */
struct Fullscreen_Menu_LoadReplay : public Fullscreen_Menu_Base {
	Fullscreen_Menu_LoadReplay();

	std::string const & filename() {return m_filename;}

	void clicked_ok();
	void clicked_delete();
	void replay_selected(uint32_t);
	void double_clicked(uint32_t);
	void fill_list();

	bool handle_key(bool down, SDL_keysym code);

private:
	void no_selection();

	uint32_t m_butw;
	uint32_t m_buth;

	UI::Button                             m_back;
	UI::Button                             m_ok;
	UI::Button                             m_delete;
	UI::Listselect<std::string>                     m_list;
	UI::Textarea                                    m_title;
	UI::Textarea                                    m_label_mapname;
	UI::Textarea                                    m_tamapname;
	UI::Textarea                                    m_label_gametime;
	UI::Textarea                                    m_tagametime;
	std::string                                     m_filename;
};


#endif
