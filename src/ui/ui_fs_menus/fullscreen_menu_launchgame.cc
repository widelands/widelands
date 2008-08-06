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

#include "fullscreen_menu_launchgame.h"

#include "fullscreen_menu_mapselect.h"
#include "game.h"
#include "gamechatpanel.h"
#include "gamecontroller.h"
#include "gamesettings.h"
#include "i18n.h"
#include "instances.h"
#include "player.h"
#include "map.h"
#include "map_loader.h"
#include "playerdescrgroup.h"
#include "profile.h"

Fullscreen_Menu_LaunchGame::Fullscreen_Menu_LaunchGame
	(GameSettingsProvider * const settings, GameController * const ctrl)
:
Fullscreen_Menu_Base("launchgamemenu.jpg"),
m_select_map
	(this,
	 550, 210, 200, 26,
	 1,
	 &Fullscreen_Menu_LaunchGame::select_map, this,
	 _("Select map"),
	 std::string(),
	 false),
m_back
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
m_title      (this, MENU_XRES / 2,  80, _("Launch Game"), Align_HCenter),
m_mapname    (this, 650,           180, std::string(),    Align_HCenter),
m_settings   (settings),
m_ctrl       (ctrl),
m_chat       (0),
m_is_scenario(false)
{

	m_title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	uint32_t y = 150;
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
		m_players[i] =
			new PlayerDescriptionGroup(this, 50, y += 30, settings, i);
}


/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void Fullscreen_Menu_LaunchGame::start()
{
	if
		(m_settings->settings().mapname.size() == 0
		 &&
		 m_settings->canChangeMap())
	{
		select_map();

		if (m_settings->settings().mapname.size() == 0)
			end_modal(0);
	}

	refresh();
}


void Fullscreen_Menu_LaunchGame::think()
{
	if (m_ctrl)
		m_ctrl->think();

	refresh();
}


/**
 * Set a new chat provider.
 *
 * This automatically creates and display a chat panel when appropriate.
 */
void Fullscreen_Menu_LaunchGame::setChatProvider(ChatProvider * const chat)
{
	delete m_chat;
	m_chat = chat ? new GameChatPanel(this, 50, 420, 480, 160, *chat) : 0;
}


/**
 * back-button has been pressed
 */
void Fullscreen_Menu_LaunchGame::back_clicked()
{
	// the following behaviour might look strange at first view, but for the
	// user it seems as if the launchgame-menu is a child of mapselect and not
	// the other way around - just end_modal(0); will be seen as bug from
	// user point of view, so we reopen the mapselect-menu.
	m_settings->setMap("","",0);
	select_map();
	if (m_settings->settings().mapname.size() == 0)
		end_modal(0);
	refresh();
}


/**
 * start-button has been pressed
 */
void Fullscreen_Menu_LaunchGame::start_clicked()
{
	if (m_settings->canLaunch())
		end_modal(1 + m_is_scenario);
}


void Fullscreen_Menu_LaunchGame::refresh()
{
	GameSettings const & settings = m_settings->settings();

	m_mapname.set_text
		(settings.mapname.size() != 0 ? settings.mapname : _("(no map)"));

	m_ok.set_enabled(m_settings->canLaunch());
	m_select_map.set_visible(m_settings->canChangeMap());
	m_select_map.set_enabled(m_settings->canChangeMap());

	if(m_settings->settings().scenario == true)
		set_scenario_values();

	// update the player description groups
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
		m_players[i]->refresh();
}

void Fullscreen_Menu_LaunchGame::select_map()
{
	if (!m_settings->canChangeMap())
		return;

	GameSettings const & settings = m_settings->settings();
	Fullscreen_Menu_MapSelect msm;
	msm.setScenarioSelectionVisible(!settings.multiplayer);
	int code = msm.run();

	if (code <= 0)
		return;

	m_is_scenario = code == 2;
	m_settings->setScenario(m_is_scenario);

	MapData const & mapdata = *msm.get_map();
	m_settings->setMap(mapdata.name, mapdata.filename, mapdata.nrplayers);
}

void Fullscreen_Menu_LaunchGame::set_scenario_values()
{
	if (m_settings->settings().mapfilename.size() == 0)
		throw wexception
				("settings()->scenario was set to true, but no map is available");
	Widelands::Map map; //  Map_Loader needs a place to put it's preload data
	Widelands::Map_Loader * const ml =
			map.get_correct_loader(m_settings->settings().mapfilename.c_str());
	map.set_filename(m_settings->settings().mapfilename.c_str());
	ml->preload_map(true);
	uint8_t nrplayers = map.get_nrplayers();
	for (uint32_t i = 0; i < nrplayers; ++i) {
		m_settings->setPlayerName (i, map.get_scenario_player_name(i+1));
		m_settings->setPlayerTribe(i, map.get_scenario_player_tribe(i+1));
	}
}
