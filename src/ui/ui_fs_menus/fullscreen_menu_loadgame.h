/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_LOADGAME_H
#define FULLSCREEN_MENU_LOADGAME_H

#include "filesystem.h"
#include "fullscreen_menu_base.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_listselect.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"

namespace Widelands {
struct Editor_Game_Base;
struct Game;
struct Map;
struct Map_Loader;
};
struct RenderTarget;

/// Select a Saved Game in Fullscreen Mode. It's a modal fullscreen menu.
struct Fullscreen_Menu_LoadGame : public Fullscreen_Menu_Base {
	Fullscreen_Menu_LoadGame(Widelands::Game &);
	~Fullscreen_Menu_LoadGame();

	const std::string & filename() {return m_filename;}

	void clicked_ok    ();
	void map_selected  (uint32_t);
	void double_clicked(uint32_t);
	void fill_list     ();

private:
	uint32_t    m_butw;
	uint32_t    m_buth;
	uint32_t    m_fs;
	std::string m_fn;

	Widelands::Game &                               m_game;
	UI::Callback_IDButton<Fullscreen_Menu_LoadGame, int32_t> m_back;
	UI::Callback_Button<Fullscreen_Menu_LoadGame>            m_ok;
	UI::Listselect<const char *>                    m_list;
	UI::Textarea                                    m_title;
	UI::Textarea                                    m_label_mapname;
	UI::Textarea                                    m_tamapname;
	UI::Textarea                                    m_label_gametime;
	UI::Textarea                                    m_tagametime;
	std::string                                     m_filename;

	filenameset_t                                   m_gamefiles;

};


#endif
