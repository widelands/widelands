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
   m_is_scenario = false;

	// Title
   UITextarea* title= new UITextarea(this, MENU_XRES/2, 140, "Launch Game", Align_HCenter);
   title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
 
	// UIButtons
	UIButton* b;

	b = new UIButton(this, 410, 356, 174, 24, 0, 0);
	b->clicked.set(this, &Fullscreen_Menu_LaunchGame::back);
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
   // Directly go selecting a map
   select_map();
}

void Fullscreen_Menu_LaunchGame::think()
{
	m_game->think();
}

/*
 * back has been pressed, clean the game up
 * so that nobody complains
 */
void Fullscreen_Menu_LaunchGame::back() {
   m_game->get_objects()->cleanup(m_game);
   end_modal(0);
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
      m_players[i]->allow_changes(true);
		m_players[i]->set_enabled(i < maxplayers);
      if(m_is_scenario && (i<maxplayers) && map) {
         // set player to the by the map given
         m_players[i]->allow_changes(false);
         m_players[i]->set_player_tribe(map->get_scenario_player_tribe(i+1));
         m_players[i]->set_player_name(map->get_scenario_player_name(i+1));
      } else if(i<maxplayers && map ) {
         std::string name="Player ";
         if((i+1)/10) name.append(1, static_cast<char>((i+1)/10 + 0x30));
         name.append(1, static_cast<char>(((i+1)%10) + 0x30));
         m_players[i]->set_player_name(name);
         m_players[i]->allow_changes(true);
      }
   }

	m_ok->set_enabled(m_game->can_start());
}

void Fullscreen_Menu_LaunchGame::select_map()
{
   // Clean all the stuff up, so we can load
   // TODO: This needs to be done properly
   m_game->get_objects()->cleanup(m_game);
   
   Fullscreen_Menu_MapSelect* msm=new Fullscreen_Menu_MapSelect(m_game);
   if(msm->run()==2)
      m_is_scenario=true;
   else 
      m_is_scenario=false;

   delete msm;
   refresh();
}

