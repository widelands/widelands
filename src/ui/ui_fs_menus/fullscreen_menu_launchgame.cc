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

#include "fullscreen_menu_launchgame.h"
#include "fullscreen_menu_mapselect.h"
#include "game.h"
#include "map.h"
#include "playerdescrgroup.h"
#include "ui_button.h"
#include "ui_textarea.h"

/*
==============================================================================

Fullscreen_Menu_LaunchGame

==============================================================================
*/

Fullscreen_Menu_LaunchGame::Fullscreen_Menu_LaunchGame(Game *g)
	: Fullscreen_Menu_Base("launchgamemenu.jpg")
{
	m_game = g;

	// Title
	new UITextarea(this, MENU_XRES/2, 140, "Launch game", Align_HCenter);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 410, 356, 174, 24, 0, 0);
	b->clickedid.set(this, &Fullscreen_Menu_LaunchGame::end_modal);
	b->set_title("Back");

	m_ok = new UIButton(this, 410, 386, 174, 24, 2, 1);
	m_ok->clickedid.set(this, &Fullscreen_Menu_LaunchGame::end_modal);
	m_ok->set_title("Start game");
	m_ok->set_enabled(false);

	// Map selection fields
	m_mapname = new UITextarea(this, 497, 184, "(no map)", Align_HCenter);
	b = new UIButton(this, 410, 200, 174, 24, 1, 0);
	b->clicked.set(this, &Fullscreen_Menu_LaunchGame::select_map);
	b->set_title("Select map");

	// Player settings
	int i;
	int y;

	y = 184;
	for(i = 1; i <= MAX_PLAYERS; i++)	{ // players start with 1, not 0
		PlayerDescriptionGroup *pdg = new PlayerDescriptionGroup(this, 30, y, m_game, i);
		pdg->changed.set(this, &Fullscreen_Menu_LaunchGame::refresh);

		m_players[i-1] = pdg;
		y += 30;
	}
}

void Fullscreen_Menu_LaunchGame::think()
{
	m_game->think();
}

void Fullscreen_Menu_LaunchGame::refresh()
{
	Map* map = m_game->get_map();
	int maxplayers = 0;

	// update the mapname
	if (map)
	{
		m_mapname->set_text(map->get_name());
		maxplayers = map->get_nrplayers();
	}
	else
		m_mapname->set_text("(no map)");

	// update the player description groups
	int i;
	for(i = 0; i < MAX_PLAYERS; i++) {
		m_players[i]->set_enabled(i < maxplayers);
   }

	m_ok->set_enabled(m_game->can_start());
}

void Fullscreen_Menu_LaunchGame::select_map()
{
   Fullscreen_Menu_MapSelect* msm=new Fullscreen_Menu_MapSelect(m_game);
   msm->run();
   delete msm;
   refresh();
}

