/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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


#include "gamecontroller.h"
#include "gamesettings.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "loadgame.h"
#include "logic/game.h"
#include "logic/instances.h"
#include "logic/map.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "mapselect.h"
#include "profile/profile.h"
#include "scripting/scripting.h"
#include "warning.h"
#include "wui/playerdescrgroup.h"

#include "launchSPG.h"

Fullscreen_Menu_LaunchSPG::Fullscreen_Menu_LaunchSPG
	(GameSettingsProvider * const settings, GameController * const ctrl,
	 bool autolaunch)
	:
	Fullscreen_Menu_Base("launchgamemenu.jpg"),

// Values for alignment and size
	m_butw (m_xres / 4),
	m_buth (m_yres * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Buttons
	m_select_map
		(this, "select_map",
		 m_xres * 7 / 10, m_yres * 3 / 10, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Fullscreen_Menu_LaunchSPG::select_map, *this,
		 _("Select map"), std::string(), false, false,
		 m_fn, m_fs),
	m_wincondition
		(this, "win_condition",
		 m_xres * 7 / 10, m_yres * 4 / 10, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Fullscreen_Menu_LaunchSPG::win_condition_clicked, *this,
		 "", std::string(), false, false,
		 m_fn, m_fs),
	m_back
		(this, "back",
		 m_xres * 7 / 10, m_yres * 9 / 20, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Fullscreen_Menu_LaunchSPG::back_clicked, *this,
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs),
	m_ok
		(this, "ok",
		 m_xres * 7 / 10, m_yres * 1 / 2, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
		 &Fullscreen_Menu_LaunchSPG::start_clicked, *this,
		 _("Start game"), std::string(), false, false,
		 m_fn, m_fs),

// Text labels
	m_title
		(this,
		 m_xres / 2, m_yres / 10,
		 _("Launch Game"), UI::Align_HCenter),
	m_mapname
		(this,
		 m_xres * 7 / 10 + m_butw / 2, m_yres * 5 / 20,
		 std::string(), UI::Align_HCenter),
	m_name
		(this,
		 m_xres * 1 / 25, m_yres * 53 / 200,
		 _("Player's name"), UI::Align_Left),
	m_type
		(this,
		 m_xres * 19 / 100, m_yres * 53 / 200,
		 _("Player's type"), UI::Align_Left),
	m_team
		(this,
		 m_xres * 31 / 100, m_yres * 53 / 200,
		 _("Team"), UI::Align_Left),
	m_tribe
		(this,
		 m_xres * 36 / 100, m_yres * 53 / 200,
		 _("Player's tribe"), UI::Align_Left),
	m_init
		(this,
		 m_xres * 51 / 100, m_yres * 53 / 200,
		 _("Start type"), UI::Align_Left),

// Variables and objects used in the menu
	m_settings     (settings),
	m_ctrl         (ctrl),
	m_is_scenario  (false)
{

	// Register win condition scripts
	m_lua = create_LuaInterface();
	m_lua->register_scripts(*g_fs, "win_conditions", "scripting/win_conditions");

	ScriptContainer sc = m_lua->get_scripts_for("win_conditions");
	container_iterate_const(ScriptContainer, sc, wc)
		m_win_conditions.push_back(wc->first);
	m_cur_wincondition = -1;
	win_condition_clicked();

	m_title  .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_mapname.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_name   .set_font(m_fn, m_fs * 4 / 5, UI_FONT_CLR_FG);
	m_type   .set_font(m_fn, m_fs * 4 / 5, UI_FONT_CLR_FG);
	m_team   .set_font(m_fn, m_fs * 4 / 5, UI_FONT_CLR_FG);
	m_tribe  .set_font(m_fn, m_fs * 4 / 5, UI_FONT_CLR_FG);
	m_init   .set_font(m_fn, m_fs * 4 / 5, UI_FONT_CLR_FG);

	uint32_t y = m_yres / 4;
	char posIco[42];
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i) {
		sprintf(posIco, "pics/fsel_editor_set_player_0%i_pos.png", i + 1);
		m_pos[i] =
			new UI::Callback_IDButton<Fullscreen_Menu_LaunchSPG, uint8_t>
				(this, "switch_to_position",
				 m_xres / 100, y += m_buth, m_yres * 17 / 500, m_yres * 17 / 500,
				 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
				 g_gr->get_picture(PicMod_Game, posIco),
				 &Fullscreen_Menu_LaunchSPG::switch_to_position, *this, i,
				 _("Switch to position"), false);
		m_players[i] =
			new PlayerDescriptionGroup
				(this,
				 m_xres / 25, y, m_xres * 16 / 25, m_yres * 17 / 500,
				 settings, i,
				 m_fn, m_fs);
	}
}

Fullscreen_Menu_LaunchSPG::~Fullscreen_Menu_LaunchSPG() {
	delete m_lua;
}

/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void Fullscreen_Menu_LaunchSPG::start()
{
	select_map();
	if (m_settings->settings().mapname.empty())
		end_modal(0); // back was pressed
}


void Fullscreen_Menu_LaunchSPG::think()
{
	if (m_ctrl)
		m_ctrl->think();

	refresh();
}


/**
 * back-button has been pressed
 */
void Fullscreen_Menu_LaunchSPG::back_clicked()
{
	//  The following behaviour might look strange at first view, but for the
	//  user it seems as if the launchgame-menu is a child of mapselect and
	//  not the other way around - just end_modal(0); will be seen as bug
	//  from user point of view, so we reopen the mapselect-menu.
	m_settings->setMap(std::string(), std::string(), 0);
	select_map();
	if (m_settings->settings().mapname.empty())
		end_modal(0);
	refresh();
}

/**
 * WinCondition button has been pressed
 */
void Fullscreen_Menu_LaunchSPG::win_condition_clicked()
{
	if (m_settings->canChangeMap()) {
		m_cur_wincondition++;
		m_cur_wincondition %= m_win_conditions.size();
		m_settings->setWinCondition(m_win_conditions[m_cur_wincondition]);
	}

	win_condition_update();
}

/**
 * update win conditions information
 */
void Fullscreen_Menu_LaunchSPG::win_condition_update() {
	if (m_settings->settings().scenario) {
		m_wincondition.set_title(_("Scenario"));
		m_wincondition.set_tooltip
			(_("Win condition is set through the scenario"));
	} else {
		boost::shared_ptr<LuaTable> t = m_lua->run_script
			("win_conditions", m_settings->getWinCondition());

		try {

			std::string n = t->get_string("name");
			std::string d = t->get_string("description");

			m_wincondition.set_title(_("Type: ") + n);
			m_wincondition.set_tooltip(d.c_str());
		} catch(LuaTableKeyError &) {
			// might be that this is not a win condition after all.
			win_condition_clicked();
		}
	}
}

/**
 * start-button has been pressed
 */
void Fullscreen_Menu_LaunchSPG::start_clicked()
{
	if (!g_fs->FileExists(m_filename))
		throw warning
			(_("File not found"),
			 _
			 	("Widelands tried to start a game with a file that could not be "
			 	 "found at given path.\n"
			 	 "The file was: %s\n"
			 	 "If this happens in a network game, the host might have selected "
			 	 "a file that you do not own. Normally such a file should be send "
			 	 "from the host to you, but perhaps the transfer was not yet "
			 	 "finnished!?!"),
			 m_filename.c_str());
	if (m_settings->canLaunch()) {
		end_modal(1 + m_is_scenario);
	}
}


/**
 * update the user interface and take care about the visibility of
 * buttons and text.
 */
void Fullscreen_Menu_LaunchSPG::refresh()
{
	GameSettings const & settings = m_settings->settings();

	m_mapname.set_text
		(settings.mapname.size() != 0 ? settings.mapname : _("(no map)"));
	m_filename = settings.mapfilename;
	m_nr_players = settings.players.size();

	m_ok.set_enabled(m_settings->canLaunch());

	m_select_map.set_visible(m_settings->canChangeMap());
	m_select_map.set_enabled(m_settings->canChangeMap());
	m_wincondition.set_enabled
		(m_settings->canChangeMap() && !settings.scenario);

	if (settings.scenario)
		set_scenario_values();

	// "Choose Position" Buttons in frond of PDG
	for (uint8_t i = 0; i < m_nr_players; ++i) {
		m_pos[i]->set_visible(true);
		PlayerSettings const & player = settings.players[i];
		if
			(player.state == PlayerSettings::stateOpen or
			 player.state == PlayerSettings::stateComputer)
			m_pos[i]->set_enabled(true);
		else
			m_pos[i]->set_enabled(false);
	}
	for (uint32_t i = m_nr_players; i < MAX_PLAYERS; ++i)
		m_pos[i]->set_visible(false);

	// update the player description groups
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
		m_players[i]->refresh();

	win_condition_update();
}


/**
 * Select a map and send all information to the user interface.
 */
void Fullscreen_Menu_LaunchSPG::select_map()
{
	if (!m_settings->canChangeMap())
		return;

	Fullscreen_Menu_MapSelect msm(Map::SP_SCENARIO);
	int code = msm.run();

	if (code <= 0) {
		// Set scenario = false, else the menu might crash when back is pressed.
		m_settings->setScenario(false);
		return;  // back was pressed
	}

	m_is_scenario = code == 2;
	m_settings->setScenario(m_is_scenario);

	MapData const & mapdata = *msm.get_map();
	m_nr_players = mapdata.nrplayers;

	safe_place_for_host(m_nr_players);
	m_settings->setMap(mapdata.name, mapdata.filename, m_nr_players);
}


/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void Fullscreen_Menu_LaunchSPG::set_scenario_values()
{
	if (m_settings->settings().mapfilename.empty())
		throw wexception
				("settings()->scenario was set to true, but no map is available");
	Widelands::Map map; //  Map_Loader needs a place to put it's preload data
	Widelands::Map_Loader * const ml =
		map.get_correct_loader(m_settings->settings().mapfilename.c_str());
	map.set_filename(m_settings->settings().mapfilename.c_str());
	ml->preload_map(true);
	Widelands::Player_Number const nrplayers = map.get_nrplayers();
	for (uint8_t i = 0; i < nrplayers; ++i) {
		m_settings->setPlayerName (i, map.get_scenario_player_name (i + 1));
		m_settings->setPlayerTribe(i, map.get_scenario_player_tribe(i + 1));
	}
}

/**
 * Called when a position-button was clicked.
 */
void Fullscreen_Menu_LaunchSPG::switch_to_position(uint8_t const pos)
{
	GameSettings settings = m_settings->settings();
	PlayerSettings const position = settings.players.at(pos);
	PlayerSettings const player   = settings.players.at(settings.playernum);
	if
		(pos < m_nr_players and
		 (position.state == PlayerSettings::stateOpen or
		  position.state == PlayerSettings::stateComputer))
	{
		m_settings->setPlayer(pos, player);
		m_settings->setPlayer(settings.playernum, position);
		m_settings->setPlayerNumber(pos);
	}
}


/**
 * Check to avoid segfaults, if the player changes a map with less player
 * positions while being on a later invalid position.
 */
void Fullscreen_Menu_LaunchSPG::safe_place_for_host
	(uint8_t const newplayernumber)
{
	GameSettings settings = m_settings->settings();

	// Check whether the host would still keep a valid position and return if
	// yes.
	if
		(settings.playernum == UserSettings::none() or
		 settings.playernum < newplayernumber)
		return;

	// Check if a still valid place is open.
	for (uint8_t i = 0; i < newplayernumber; ++i) {
		PlayerSettings position = settings.players.at(i);
		if (position.state == PlayerSettings::stateOpen) {
			switch_to_position(i);
			return;
		}
	}

	// Kick player 1 and take the position
	m_settings->setPlayerState(0, PlayerSettings::stateClosed);
	m_settings->setPlayerState(0, PlayerSettings::stateOpen);
	switch_to_position(0);
}
