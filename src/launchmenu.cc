/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "graphic.h"
#include "input.h"
#include "ui.h"
#include "fileloc.h"
#include "cursor.h"
#include "font.h"
#include "mainmenue.h"
#include "menuecommon.h"
#include "game.h"
#include "mapselectmenue.h"

/*
==============================================================================

LaunchGameMenu

==============================================================================
*/


class LaunchGameMenu : public BaseMenu {
	Game	*m_game;

	Button	*m_ok;
	Textarea	*m_mapname;
public:
	LaunchGameMenu(Game *g);

	void think();

	void refresh();

	void select_map();
};

LaunchGameMenu::LaunchGameMenu(Game *g)
	: BaseMenu("splash.bmp")
{
	m_game = g;

	// Title
	new Textarea(this, MENU_XRES/2, 140, "Launch game", Textarea::H_CENTER);

	// Buttons
	Button* b;

	b = new Button(this, 410, 406, 174, 24, 0, 0);
	b->clickedid.set(this, &LaunchGameMenu::end_modal);
	b->set_pic(g_fh.get_string("Back", 0));

	m_ok = new Button(this, 410, 436, 174, 24, 2, 1);
	m_ok->clickedid.set(this, &LaunchGameMenu::end_modal);
	m_ok->set_pic(g_fh.get_string("Start game", 0));
	m_ok->set_enabled(false);

	// Map selection fields
	m_mapname = new Textarea(this, 497, 180, "(no map)", Textarea::H_CENTER);
	b = new Button(this, 410, 200, 174, 24, 1, 0);
	b->clicked.set(this, &LaunchGameMenu::select_map);
	b->set_pic(g_fh.get_string("Select map", 0));
}

void LaunchGameMenu::think()
{
	m_game->think();
}

void LaunchGameMenu::refresh()
{
	const char *mapname = m_game->get_mapname();
	if (mapname) {
		m_mapname->set_text(mapname);
		m_ok->set_enabled(true);
	} else {
		m_mapname->set_text("(no map)");
		m_ok->set_enabled(false);
	}
}

void LaunchGameMenu::select_map()
{
	map_select_menue(m_game);
	refresh();
}

/** void launch_game_menu(void)
 *
 * In the Launch Game menu, players can set the game settings, go to
 * map selection etc..
 */
bool launch_game_menu(Game *g)
{
	LaunchGameMenu *lgm = new LaunchGameMenu(g);
	int code = lgm->run();
	delete lgm;

	return (code > 0) ? true : false;
}
