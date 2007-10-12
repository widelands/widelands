/*
 * Copyright (C) 2002, 2006-2007 by the Widelands Development Team
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
#include "i18n.h"
#include "instances.h"
#include "player.h"
#include "network.h"
#include "map.h"
#include "playerdescrgroup.h"
#include <stdint.h>


Fullscreen_Menu_LaunchGame::Fullscreen_Menu_LaunchGame(Game *g, NetGame* ng, Map_Loader** ml)
:
Fullscreen_Menu_Base("launchgamemenu.jpg"),
m_game(g),
m_netgame(ng),
m_ml(ml),

	// Title
title(this, MENU_XRES/2, 120, _("Launch Game"), Align_HCenter),

back
(this,
 550, 450, 200, 26,
 0,
 &Fullscreen_Menu_LaunchGame::back_clicked, this,
 _("Back")),

m_ok
(this,
 550, 480, 200, 26,
 2,
 &Fullscreen_Menu_LaunchGame::start_clicked, this,
 _("Start game"),
 std::string(),
 false),

	// Map selection fields
m_mapname(this, 650, 250, "(no map)", Align_HCenter),
m_select_map
(this,
 550, 280, 200, 26,
 1,
 &Fullscreen_Menu_LaunchGame::select_map, this,
 _("Select map"),
 std::string(),
 not ng or ng->is_host()),


m_is_scenario(false)

{
	title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	m_ok.set_visible(not ng or ng->is_host());
// Player settings
	int32_t y;

	y = 250;
	iterate_player_numbers(p, MAX_PLAYERS) {
		PlayerDescriptionGroup *pdg = new PlayerDescriptionGroup
			(this,
			 50, y,
			 m_game, p, m_netgame && m_netgame->get_playernum() == p);
		pdg->changed.set(this, &Fullscreen_Menu_LaunchGame::refresh);

		m_players[p - 1] = pdg;
		y += 30;

		if (m_netgame!=0)
		    m_netgame->set_player_description_group (p, pdg);
	}

	if (not m_netgame) m_players[0]->set_player_type(Player::Local);
}


/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void Fullscreen_Menu_LaunchGame::start()
{
	if (m_netgame == 0 || m_netgame->is_host()) {
		select_map();

		if (!m_game->get_map())
			back_clicked();
	}
}


void Fullscreen_Menu_LaunchGame::think()
{
	m_game->think();
}

/*
 * back-button has been pressed
 * */
void Fullscreen_Menu_LaunchGame::back_clicked()
{
	m_game->cleanup_objects();
	end_modal(0);
}


/*
 * start-button has been pressed
 * */
void Fullscreen_Menu_LaunchGame::start_clicked()
{
	end_modal(m_is_scenario?2:1);
}


void Fullscreen_Menu_LaunchGame::refresh()
{
	Map* map = m_game->get_map();
	uint32_t maxplayers = 0;

	// update the mapname
	if (map)
	{
		m_mapname.set_text(map->get_name());
		maxplayers = map->get_nrplayers();
	}
	else m_mapname.set_text(_("(no map)"));

	// update the player description groups
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i) {
		m_players[i]->allow_changes(PlayerDescriptionGroup::CHANGE_EVERYTHING);
		m_players[i]->set_enabled(i < maxplayers);
		if (m_is_scenario && (i<maxplayers) && map) {
			// set player to the by the map given
			m_players[i]->allow_changes(PlayerDescriptionGroup::CHANGE_NOTHING);
			m_players[i]->set_player_tribe(map->get_scenario_player_tribe(i+1));
			m_players[i]->set_player_name(map->get_scenario_player_name(i+1));
		} else if (i<maxplayers && map) {
			std::string name=_("Player ");
			if ((i+1)/10) name.append(1, static_cast<char>((i+1)/10 + 0x30));
			name.append(1, static_cast<char>(((i+1)%10) + 0x30));
			m_players[i]->set_player_name(name);
			m_players[i]->allow_changes(PlayerDescriptionGroup::CHANGE_EVERYTHING);
		}

		if (m_netgame!=0) {
			int32_t allow=PlayerDescriptionGroup::CHANGE_NOTHING;

			if (m_netgame->is_host() && i>0)
				allow|=PlayerDescriptionGroup::CHANGE_ENABLED;

			if (m_netgame->get_playernum()==i+1)
				allow|=PlayerDescriptionGroup::CHANGE_TRIBE;

			m_players[i]->allow_changes
				(static_cast<const PlayerDescriptionGroup::changemode_t>(allow));
		}
	}

	m_ok.set_enabled(m_game->can_start());
}

void Fullscreen_Menu_LaunchGame::select_map()
{
	{
		Fullscreen_Menu_MapSelect msm(m_game, m_ml);
		m_is_scenario = msm.run() == 2;
	}

	if (m_netgame)
		static_cast<NetHost*>(m_netgame)->update_map();

	refresh();
}
