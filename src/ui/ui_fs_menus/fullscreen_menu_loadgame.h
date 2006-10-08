/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#ifndef __S__LOAD_GAMEMENUE_H
#define __S__LOAD_GAMEMENUE_H

#include "filesystem.h"
#include "fullscreen_menu_base.h"

class Editor_Game_Base;
class Game;
class Map;
class Map_Loader;
class RenderTarget;
class UIButton;
class UIListselect;
class UIMultiline_Textarea;
class UITextarea;

/**
 * Select a Saved Game in Fullscreen Mode. It's a modal fullscreen menu
 */
class Fullscreen_Menu_LoadGame : public Fullscreen_Menu_Base {
	Game* game;

	UIListselect *list;
	UITextarea *tamapname;
	UITextarea *tagametime;
	UIButton *m_ok;
   std::string m_filename;

	filenameset_t	m_gamefiles;

public:
	Fullscreen_Menu_LoadGame(Game *g, bool is_singleplayer);
	~Fullscreen_Menu_LoadGame();

	const char *get_gamename() { return m_filename.c_str(); }

	void ok();
	void map_selected(int id);
   void double_clicked(int);
   void fill_list(void);
};


#endif
