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
#include "multiplayersetupbox.h"
#include "profile/profile.h"
#include "scripting/scripting.h"
#include "warning.h"
#include "wui/gamechatpanel.h"

#include "launchMPG.h"

Fullscreen_Menu_LaunchMPG::Fullscreen_Menu_LaunchMPG
	(GameSettingsProvider * const settings, GameController * const ctrl,
	 uint32_t usernum, bool autolaunch)
	:
	Fullscreen_Menu_Base("launchgamemenu.jpg"),

// Values for alignment and size
	m_butw (m_xres / 4),
	m_buth (m_yres * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

	m_usernum(usernum),

// Buttons
	m_back
		(this, "back",
		 m_xres * 7 / 10, m_yres * 218 / 240, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Fullscreen_Menu_LaunchMPG::back_clicked, *this,
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs),
	m_wincondition
		(this, "win_condition",
		 m_xres * 7 / 10, m_yres * 4 / 10, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Fullscreen_Menu_LaunchMPG::win_condition_clicked, *this,
		 "", std::string(), false, false,
		 m_fn, m_fs),

// Text labels
	m_title
		(this,
		 m_xres / 2, m_yres / 18,
		 _("Multiplayer Game Setup"), UI::Align_HCenter),
	m_mapname
		(this,
		 m_xres * 7 / 10 + m_butw / 2, m_yres * 5 / 20,
		 std::string(), UI::Align_HCenter),
	m_lobby
		(this,
		 m_xres * 7 / 10, m_yres * 11 / 20),

// Variables and objects used in the menu
	m_settings     (settings),
	m_ctrl         (ctrl),
	m_chat         (0),
	m_is_scenario  (false),
	m_is_savegame  (false),
	m_autolaunch   (autolaunch)
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

	m_mpsb =
		new MultiPlayerSetupBox
			(this,
			 m_xres / 20, m_yres / 10, m_xres * 9 / 10, m_yres * 6 / 11,
			 settings, m_usernum, m_butw, m_buth, m_fn, m_fs);

	m_lobby_list =
		new UI::Listselect<int32_t>
			(this, m_xres * 7 / 10, m_yres * 13 / 20, m_butw, m_yres * 5 / 20);
	m_lobby_list->set_font(m_fn, m_fs);
	m_lobby.set_text(_("Lobby:"));
	m_lobby.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
}

Fullscreen_Menu_LaunchMPG::~Fullscreen_Menu_LaunchMPG() {
	delete m_lua;
}


void Fullscreen_Menu_LaunchMPG::think()
{
	if (m_ctrl)
		m_ctrl->think();

	refresh();
}


/**
 * Set a new chat provider.
 *
 * This automatically creates and displays a chat panel when appropriate.
 */
void Fullscreen_Menu_LaunchMPG::setChatProvider(ChatProvider & chat)
{
	delete m_chat;
	m_chat =
		new GameChatPanel
			(this,
			 m_xres / 20, m_yres * 13 / 20, m_xres * 51 / 80, m_yres * 3 / 10,
			 chat);
	// For better readability
	m_chat->set_bg_color(RGBColor(50, 50, 50));
}


/**
 * back-button has been pressed
 */
void Fullscreen_Menu_LaunchMPG::back_clicked()
{
	end_modal(0);
}

/**
 * WinCondition button has been pressed
 */
void Fullscreen_Menu_LaunchMPG::win_condition_clicked()
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
void Fullscreen_Menu_LaunchMPG::win_condition_update() {
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
void Fullscreen_Menu_LaunchMPG::start_clicked()
{
	// TODO move to MPSB
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
		if (!m_is_savegame)
			end_modal(1 + m_is_scenario);
		else
			end_modal(3 + m_is_scenario);
	}
}


/**
 * update the user interface and take care about the visibility of
 * buttons and text.
 */
void Fullscreen_Menu_LaunchMPG::refresh()
{
	GameSettings const & settings = m_settings->settings();

	m_mpsb->refresh();

	m_mapname.set_text
		(settings.mapname.size() != 0 ? settings.mapname : _("(no map)"));
	m_is_savegame = settings.savegame;
	m_filename = settings.mapfilename;
	m_nr_players = settings.players.size();

	bool launch = m_settings->canLaunch();
	//check if we want to autolaunch
	if (m_autolaunch && launch)
		start_clicked();

	m_wincondition.set_enabled
		(m_settings->canChangeMap() && !m_is_savegame && !settings.scenario);

	if (settings.scenario)
		set_scenario_values();

	if (m_is_savegame) {
		if (m_filename != m_filename_proof)
			// load all playerdata from savegame
			load_previous_playerdata();
	}

	// Care about Multiplayer clients, lobby and players
	// As well as win_conditions
	m_lobby_list->clear();
	container_iterate_const(std::vector<UserSettings>, settings.users, i)
		m_lobby_list->add(i.current->name.c_str(), 0);
	win_condition_update();
}


/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void Fullscreen_Menu_LaunchMPG::set_scenario_values()
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
		if (!m_settings->settings().multiplayer)
			m_settings->setPlayerName (i, map.get_scenario_player_name (i + 1));
		m_settings->setPlayerTribe(i, map.get_scenario_player_tribe(i + 1));
	}
}

/**
 * Called when a position-button was clicked.
 */
void Fullscreen_Menu_LaunchMPG::switch_to_position(uint8_t const pos)
{
	GameSettings settings = m_settings->settings();

	// Check if pos == current player position, if yes send player to lobby
	// This is only possible in multiplayer games.
	if ((settings.playernum == pos) & settings.multiplayer) {
		m_settings->setPlayerState(pos, PlayerSettings::stateOpen);
		m_settings->setPlayerNumber(UserSettings::none());
		return;
	}

	//  Check if current player position is none. If yes, we just assign the
	//  player to the position. This is only possible in multiplayer games.
	if (settings.playernum == UserSettings::none()) {
		if (!settings.multiplayer)
			throw wexception("player position is none in non-multiplayer game");
		if
			(pos < m_nr_players and
			 settings.players.at(pos).state == PlayerSettings::stateOpen)
		{
			m_settings->setPlayerState(pos, PlayerSettings::stateHuman);
			m_settings->setPlayerNumber(pos);
			m_settings->setPlayerName
				(pos, settings.users.at(settings.usernum).name);
		}
		return;
	}

	PlayerSettings const position = settings.players.at(pos);
	PlayerSettings const player   = settings.players.at(settings.playernum);
	if
		(pos < m_nr_players and
		 (position.state == PlayerSettings::stateOpen or
		  (position.state == PlayerSettings::stateComputer and
		   !settings.multiplayer)))
	{
		m_settings->setPlayer(pos, player);
		m_settings->setPlayer(settings.playernum, position);
		m_settings->setPlayerNumber(pos);
	}
}

/**
 * load all playerdata from savegame
 */
void Fullscreen_Menu_LaunchMPG::load_previous_playerdata()
{
	FileSystem & l_fs = g_fs->MakeSubFileSystem(m_filename.c_str());
	Profile prof;
	prof.read("map/player_names", 0, l_fs);
	std::string strbuf;
	char buf[32];

	int8_t i = 1;
	for (; i <= m_nr_players; ++i) {
		strbuf = std::string();
		snprintf(buf, sizeof(buf), "player_%i", i);
		Section & s = prof.get_safe_section(buf);
		m_player_save_name [i - 1] = s.get_string("name");
		m_player_save_tribe[i - 1] = s.get_string("tribe");

		if (m_player_save_tribe[i - 1].empty())
			continue; // if tribe is empty, the player does not exist

		// get translated tribename
		strbuf = "tribes/" + m_player_save_tribe[i - 1];
		strbuf += "/conf";
		Profile tribe(strbuf.c_str(), 0, "tribe_" + m_player_save_tribe[i - 1]);
		Section & global = tribe.get_safe_section("tribe");
		m_player_save_tribe[i - 1] = global.get_safe_string("name");
	}
	m_filename_proof = m_filename;
}


/**
 * Check to avoid segfaults, if the player changes a map with less player
 * positions while being on a later invalid position.
 */
void Fullscreen_Menu_LaunchMPG::safe_place_for_host
	(uint8_t const newplayernumber)
{
	GameSettings settings = m_settings->settings();

	// Check whether the host would still keep a valid position and return if
	// yes.
	if
		(settings.playernum == UserSettings::none() or
		 settings.playernum < newplayernumber)
		return;

	if (settings.multiplayer) {
		m_settings->setPlayerNumber(UserSettings::none());
		return;
	}

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
