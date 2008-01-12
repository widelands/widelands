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

#ifndef __S__LAUNCH_GAME_MENU_H
#define __S__LAUNCH_GAME_MENU_H

#include "constants.h"
#include "fullscreen_menu_base.h"

#include "ui_button.h"
#include "ui_textarea.h"

namespace Widelands {struct Game;
struct Map_Loader;
};
class NetGame;
class PlayerDescriptionGroup;

/**
 * Fullscreen menu to select map and to set map options.
 */
struct Fullscreen_Menu_LaunchGame : public Fullscreen_Menu_Base {
	Fullscreen_Menu_LaunchGame
		(Widelands::Game *, NetGame *, Widelands::Map_Loader * *);

	void start();
	void think();

	void refresh();

	void select_map();
	void back_clicked();
	void start_clicked();

private:
	Widelands::Game                      * m_game;
	NetGame                              * m_netgame;
	Widelands::Map_Loader *              * m_ml;

	UI::Textarea                           title;
	UI::Button<Fullscreen_Menu_LaunchGame> back;
	UI::Button<Fullscreen_Menu_LaunchGame> m_ok;
	UI::Textarea                           m_mapname;
	UI::Button<Fullscreen_Menu_LaunchGame> m_select_map;
	PlayerDescriptionGroup* m_players[MAX_PLAYERS];

	bool                    m_is_scenario;
};


#endif
