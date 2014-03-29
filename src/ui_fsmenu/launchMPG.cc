/*
 * Copyright (C) 2002, 2006-2012 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "ui_fsmenu/launchMPG.h"

#include <boost/format.hpp>
#include <libintl.h>

#include "gamecontroller.h"
#include "gamesettings.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/instances.h"
#include "logic/map.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "profile/profile.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/mapselect.h"
#include "warning.h"
#include "wui/gamechatpanel.h"
#include "wui/multiplayersetupgroup.h"

using boost::format;

/// Simple user interaction window for selecting either map, save or cancel
struct MapOrSaveSelectionWindow : public UI::Window {
	MapOrSaveSelectionWindow
		(UI::Panel * parent, GameController * gc, uint32_t w, uint32_t h,
		 UI::Font * font)
	:
	/** TRANSLATORS: Dialog box title for selecting between map or saved game for new multiplayer game */
	Window(parent, "selection_window", 0, 0, w, h, _("Please select")),
	m_ctrl(gc)
	{
		center_to_parent();

		uint32_t y     = get_inner_h() / 10;
		uint32_t space = y;
		uint32_t butw  = get_inner_w() - 2 * space;
		uint32_t buth  = (get_inner_h() - 2 * space) / 5;
		UI::Button * btn = new UI::Button
			(this, "map",
			 space, y, butw, buth,
			 g_gr->images().get("pics/but0.png"),
			 _("Map"), _("Select a map"), true, false);
		btn->sigclicked.connect
			(boost::bind
				 (&MapOrSaveSelectionWindow::pressedButton, boost::ref(*this), 1));
		btn->set_font(font);

		btn = new UI::Button
			(this, "saved_game",
			 space, y + buth + space, butw, buth,
			 g_gr->images().get("pics/but0.png"),
			 _("Saved game"), _("Select a saved game"), true, false);
		btn->sigclicked.connect
			(boost::bind
				 (&MapOrSaveSelectionWindow::pressedButton, boost::ref(*this), 2));
		btn->set_font(font);

		btn = new UI::Button
			(this, "cancel",
			 space + butw / 4, y + 3 * buth + 2 * space, butw / 2, buth,
			 g_gr->images().get("pics/but1.png"),
			 _("Cancel"), _("Cancel selection"), true, false);
		btn->sigclicked.connect
			(boost::bind
				 (&MapOrSaveSelectionWindow::pressedButton, boost::ref(*this), 0));
		btn->set_font(font);
	}


	void think() override {
		if (m_ctrl)
			m_ctrl->think();
	}

	void pressedButton(uint8_t i) {
		end_modal(i);
	}
	private:
		GameController * m_ctrl;
};

Fullscreen_Menu_LaunchMPG::Fullscreen_Menu_LaunchMPG
	(GameSettingsProvider * const settings, GameController * const ctrl)
	:
	Fullscreen_Menu_Base("launchMPGmenu.jpg"),

// Values for alignment and size
	m_butw (get_w() / 4),
	m_buth (get_h() * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Buttons
	m_change_map_or_save
		(this, "change_map_or_save",
		 get_w() * 37 / 50 + m_butw - m_buth, get_h() * 3 / 20, m_buth, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/menu_toggle_minimap.png"),
		 _("Change map or saved game"), false, false),
	m_ok
		(this, "ok",
		 get_w() * 37 / 50, get_h() * 12 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("Start game"), std::string(), false, false),
	m_back
		(this, "back",
		 get_w() * 37 / 50, get_h() * 218 / 240, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),
	m_wincondition
		(this, "win_condition",
		 get_w() * 37 / 50, get_h() * 11 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 "", std::string(), false, false),
	m_help_button
		(this, "help",
		 get_w() * 37 / 50 + m_butw - m_buth, get_h() / 100, m_buth, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/menu_help.png"),
		 _("Show the help window"), true, false),

// Text labels
	m_title
		(this,
		 get_w() / 2, get_h() / 25,
		 _("Multiplayer Game Setup"), UI::Align_HCenter),
	m_mapname
		(this,
		 get_w() * 37 / 50, get_h() * 3 / 20,
		 std::string()),
	m_clients
		(this,
		 // (get_w() * 57 / 80) is the width of the MultiPlayerSetupGroup
		 get_w() / 50, get_h() / 10, (get_w() * 57 / 80) / 3, get_h() / 10,
		 _("Clients"), UI::Align_HCenter),
	m_players
		(this,
		 get_w() / 50 + (get_w() * 57 / 80) * 6 / 15, get_h() / 10, (get_w() * 57 / 80) * 9 / 15, get_h() / 10,
		 _("Players"), UI::Align_HCenter),
	m_map
		(this,
		 get_w() * 37 / 50, get_h() / 10, m_butw, get_h() / 10,
		 _("Map"), UI::Align_HCenter),
	m_wincondition_type
		(this,
		 get_w() * 37 / 50 + (m_butw / 2), get_h() * 10 / 20,
		 _("Type of game"), UI::Align_HCenter),

	m_map_info(this, get_w() * 37 / 50, get_h() * 2 / 10, m_butw, get_h() * 23 / 80),
	m_client_info(this, get_w() * 37 / 50, get_h() * 13 / 20, m_butw, get_h() * 5 / 20),
	m_help(nullptr),

// Variables and objects used in the menu
	m_settings     (settings),
	m_ctrl         (ctrl),
	m_chat         (nullptr)
{
	m_change_map_or_save.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_LaunchMPG::change_map_or_save, boost::ref(*this)));
	m_ok.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_LaunchMPG::start_clicked, boost::ref(*this)));
	m_back.sigclicked.connect(boost::bind(&Fullscreen_Menu_LaunchMPG::back_clicked, boost::ref(*this)));
	m_wincondition.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_LaunchMPG::win_condition_clicked,
			  boost::ref(*this)));
	m_help_button.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_LaunchMPG::help_clicked,
			  boost::ref(*this)));

	m_back.set_font(font_small());
	m_ok.set_font(font_small());
	m_wincondition.set_font(font_small());
	m_help_button.set_font(font_small());
	m_change_map_or_save.set_font(font_small());
	m_wincondition_type.set_textstyle(ts_small());

	m_lua = new LuaInterface();
	win_condition_clicked();

	m_title      .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_mapname    .set_font(m_fn, m_fs, RGBColor(255, 255, 127));
	m_clients    .set_font(m_fn, m_fs, RGBColor(0, 255, 0));
	m_players    .set_font(m_fn, m_fs, RGBColor(0, 255, 0));
	m_map        .set_font(m_fn, m_fs, RGBColor(0, 255, 0));
	m_client_info.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_map_info   .set_font(m_fn, m_fs, UI_FONT_CLR_FG);

	m_mapname .set_text(_("(no map)"));
	m_map_info.set_text(_("The host has not yet selected a map or saved game."));

	m_mpsg =
		new MultiPlayerSetupGroup
			(this,
			 get_w() / 50, get_h() / 8, get_w() * 57 / 80, get_h() / 2,
			 settings, m_butw, m_buth, m_fn, m_fs);

	// If we are the host, open the map or save selection menu at startup
	if (m_settings->settings().usernum == 0 && m_settings->settings().mapname.empty())
	{
		change_map_or_save();
		// Try to associate the host with the first player
		if (m_settings->settings().players.size() > 0)
			m_settings->setPlayerNumber(0);
	}
}

Fullscreen_Menu_LaunchMPG::~Fullscreen_Menu_LaunchMPG() {
	delete m_lua;
	delete m_mpsg;
	if (m_help)
		delete m_help;
	delete m_chat;
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
	m_chat = new GameChatPanel
		(this, get_w() / 50, get_h() * 13 / 20, get_w() * 57 / 80, get_h() * 3 / 10, chat);
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
	m_settings->nextWinCondition();
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
	} else if (m_settings->settings().savegame) {
		m_wincondition.set_title(_("Savegame"));
		m_wincondition.set_tooltip
			(_("The game is a saved game – the win condition was set before."));
	} else {
		std::unique_ptr<LuaTable> t = m_lua->run_script(m_settings->getWinConditionScript());

		try {
			std::string name = t->get_string("name");
			std::string descr = t->get_string("description");

			m_wincondition.set_title(name);
			m_wincondition.set_tooltip(descr.c_str());
		} catch (LuaTableKeyError &) {
			// might be that this is not a win condition after all.
			win_condition_clicked();
		}
	}
}

/// Opens a popup window to select a map or saved game
void Fullscreen_Menu_LaunchMPG::change_map_or_save() {
	MapOrSaveSelectionWindow selection_window
		(this, m_ctrl, get_w() / 3, get_h() / 4, font_small());
	switch (selection_window.run()) {
		case 1:
			select_map();
			break;
		case 2:
			select_saved_game();
			break;
		default:
			return;
	}
}

/**
 * Select a map and send all information to the user interface.
 */
void Fullscreen_Menu_LaunchMPG::select_map() {
	if (!m_settings->canChangeMap())
		return;

	Fullscreen_Menu_MapSelect msm(m_settings, m_ctrl);
	int code = msm.run();

	if (code <= 0) {
		// Set scenario = false, else the menu might crash when back is pressed.
		m_settings->setScenario(false);
		return;  // back was pressed
	}

	m_settings->setScenario(code == 2);

	const MapData & mapdata = *msm.get_map();
	m_nr_players = mapdata.nrplayers;

	// If the same map was selected again, maybe the state of the "scenario" check box was changed
	// So we should recheck all map predefined values,
	// which is done in refresh(), if m_filename_proof is different to settings.mapfilename -> dummy rename
	if (mapdata.filename == m_filename_proof)
		m_filename_proof = m_filename_proof + "new";

	m_settings->setMap(mapdata.name, mapdata.filename, m_nr_players);
}

/**
 * Select a multi player saved game and send all information to the user
 * interface.
 */
void Fullscreen_Menu_LaunchMPG::select_saved_game() {
	if (!m_settings->canChangeMap())
		return;

	Widelands::Game game; // The place all data is saved to.
	Fullscreen_Menu_LoadGame lsgm(game, m_settings, m_ctrl);
	int code = lsgm.run();

	if (code <= 0)
		return; // back was pressed

	// Saved game was selected - therefore not a scenario
	m_settings->setScenario(false);

	std::string filename = lsgm.filename();

	if (g_fs->FileExists(filename.c_str())) {
		// Read the needed data from file "elemental" of the used map.
		std::unique_ptr<FileSystem> l_fs(g_fs->MakeSubFileSystem(filename.c_str()));
		Profile prof;
		prof.read("map/elemental", nullptr, *l_fs);
		Section & s = prof.get_safe_section("global");

		std::string mapname = s.get_safe_string("name");
		m_nr_players = s.get_safe_int("nr_players");

		m_settings->setMap(mapname, filename, m_nr_players, true);

		// Check for sendability
		if (g_fs->IsDirectory(filename)) {
			// Send a warning
			UI::WLMessageBox warning
				(this, _("Saved game is directory"),
				_
				("WARNING:\n"
					"The saved game you selected is a directory."
					" This happens if you set the option ‘nozip’ to "
					"true or manually unzipped the saved game.\n"
					"Widelands is not able to transfer directory structures to the clients,"
					" please select another saved game or zip the directories’ content."),
				UI::WLMessageBox::OK);
			warning.run();
		}
	} else {
		if (!m_settings || m_settings->settings().saved_games.empty())
			throw wexception("A file was selected, that is not available to the client");
		// this file is obviously a file from the dedicated server's saved games pool not available locally.
		for (uint32_t i = 0; i < m_settings->settings().saved_games.size(); ++i)
			if (m_settings->settings().saved_games.at(i).path == filename) {
				m_settings->setMap(filename, filename, m_settings->settings().saved_games.at(i).players, true);
				return;
			}
		throw wexception("The selected file could not be found in the pool of dedicated saved games.");
	}
}

/**
 * start-button has been pressed
 */
void Fullscreen_Menu_LaunchMPG::start_clicked()
{
	if (!g_fs->FileExists(m_settings->settings().mapfilename))
		throw warning
			(_("File not found"),
			 _
			 	("Widelands tried to start a game with a file that could not be "
			 	 "found at the given path.\n"
			 	 "The file was: %s\n"
			 	 "If this happens, the host might have selected a file that you do "
			 	 "not own. Normally, such a file should be sent from the host to "
			 	 "you, but perhaps the transfer was not yet finished!?!"),
			 m_settings->settings().mapfilename.c_str());
	if (m_settings->canLaunch())
		end_modal(1);
}


/**
 * update the user interface and take care about the visibility of
 * buttons and text.
 */
void Fullscreen_Menu_LaunchMPG::refresh()
{
	const GameSettings & settings = m_settings->settings();

	if (settings.mapfilename != m_filename_proof) {
		if (!g_fs->FileExists(settings.mapfilename)) {
			m_client_info.set_font(m_fn, m_fs, UI_FONT_CLR_WARNING);
			m_client_info.set_text
				(_("The selected file can not be found. If it is not automatically "
				   "transferred to you, please write to the host about this problem."));
		} else {
			// Reset font color
			m_client_info.set_font(m_fn, m_fs, UI_FONT_CLR_FG);

			// Update local nr of players - needed for the client UI
			m_nr_players = settings.players.size();

			// Care about the newly selected file. This has to be done here and not
			// after selection of a new map / saved game, as the clients user
			// interface can only notice the change after the host broadcasted it.
			if (settings.savegame)
				load_previous_playerdata();
			else {
				load_map_info();
				if (settings.scenario)
					set_scenario_values();
			}
			//Try to translate the map name.
			//This will work on every official map as expected
			//and 'fail silently' (not find a translation) for already translated campaign map names.
			//It will also translate 'false-positively' on any user-made map which shares a name with
			//the official maps, but this should not be a problem to worry about.
			i18n::Textdomain td("maps");
			m_mapname.set_text(_(settings.mapname));
		}
	} else {
		// Write client infos
		std::string temp =
			(settings.playernum > -1) && (settings.playernum < MAX_PLAYERS)
			?
			(format(_("Player %i")) % (settings.playernum + 1)).str()
			:
			_("Spectator");
		temp  = (format(_("At the moment you are %s")) % temp.c_str()).str() + "\n\n";
		temp += _("Click on the ‘?’ in the top right corner to get help.");
		m_client_info.set_text(temp);
	}

	m_ok.set_enabled(m_settings->canLaunch());

	m_change_map_or_save.set_enabled(m_settings->canChangeMap());
	m_change_map_or_save.set_visible(m_settings->canChangeMap());

	m_wincondition.set_enabled
		(m_settings->canChangeMap() && !settings.savegame && !settings.scenario);

	win_condition_update();

	// Update the multi player setup group
	m_mpsg->refresh();
}


/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void Fullscreen_Menu_LaunchMPG::set_scenario_values()
{
	const GameSettings & settings = m_settings->settings();
	if (settings.mapfilename.empty())
		throw wexception
			("settings()->scenario was set to true, but no map is available");
	Widelands::Map map; //  Map_Loader needs a place to put it's preload data
	std::unique_ptr<Widelands::Map_Loader> ml(map.get_correct_loader(settings.mapfilename));
	map.set_filename(settings.mapfilename.c_str());
	ml->preload_map(true);
	Widelands::Player_Number const nrplayers = map.get_nrplayers();
	for (uint8_t i = 0; i < nrplayers; ++i) {
		m_settings->setPlayerTribe    (i, map.get_scenario_player_tribe    (i + 1));
		m_settings->setPlayerCloseable(i, map.get_scenario_player_closeable(i + 1));
		std::string ai(map.get_scenario_player_ai(i + 1));
		if (ai.size() > 0) {
			m_settings->setPlayerState(i, PlayerSettings::stateComputer);
			m_settings->setPlayerAI   (i, ai);
		} else if
			(settings.players.at(i).state != PlayerSettings::stateHuman
			 &&
			 settings.players.at(i).state != PlayerSettings::stateOpen)
		{
			m_settings->setPlayerState(i, PlayerSettings::stateOpen);
		}
	}
}

/**
 * load all playerdata from savegame and update UI accordingly
 */
void Fullscreen_Menu_LaunchMPG::load_previous_playerdata()
{
	std::unique_ptr<FileSystem> l_fs(g_fs->MakeSubFileSystem(m_settings->settings().mapfilename.c_str()));
	Profile prof;
	prof.read("map/player_names", nullptr, *l_fs);
	std::string strbuf;
	std::string infotext = _("Saved players are:");
	std::string player_save_name [MAX_PLAYERS];
	std::string player_save_tribe[MAX_PLAYERS];
	std::string player_save_ai   [MAX_PLAYERS];
	char buf[32];

	uint8_t i = 1;
	for (; i <= m_nr_players; ++i) {
		infotext += "\n* ";
		strbuf = std::string();
		snprintf(buf, sizeof(buf), "player_%u", i);
		Section & s = prof.get_safe_section(buf);
		player_save_name [i - 1] = s.get_string("name");
		player_save_tribe[i - 1] = s.get_string("tribe");
		player_save_ai   [i - 1] = s.get_string("ai");

		snprintf(buf, sizeof(buf), _("Player %u"), i);
		infotext += buf;
		if (player_save_tribe[i - 1].empty()) {
			std::string closed_string =
				(boost::format("\\<%s\\>") % _("closed")).str();
			infotext += ":\n    ";
			infotext += closed_string;
			// Close the player
			m_settings->setPlayerState(i - 1, PlayerSettings::stateClosed);
			continue; // if tribe is empty, the player does not exist
		}

		// Set team to "none" - to get the real team, we would need to load the savegame completely
		// Do we want that? No! So we just reset teams to not confuse the clients.
		m_settings->setPlayerTeam(i - 1, 0);

		if (player_save_ai[i - 1].empty()) {
			// Assure that player is open
			if (m_settings->settings().players.at(i - 1).state != PlayerSettings::stateHuman)
				m_settings->setPlayerState(i - 1, PlayerSettings::stateOpen);
		} else {
			m_settings->setPlayerState(i - 1, PlayerSettings::stateComputer);
			m_settings->setPlayerAI(i - 1, player_save_ai[i - 1]);
		}

		// Set player's tribe
		m_settings->setPlayerTribe(i - 1, player_save_tribe[i - 1]);

		// get translated tribename
		strbuf = "tribes/" + player_save_tribe[i - 1];
		strbuf += "/conf";
		Profile tribe(strbuf.c_str(), nullptr, "tribe_" + player_save_tribe[i - 1]);
		Section & global = tribe.get_safe_section("tribe");
		player_save_tribe[i - 1] = global.get_safe_string("name");
		infotext += " (";
		infotext += player_save_tribe[i - 1];
		infotext += "):\n    ";
		// Check if this is a list of names, or just one name:
		if (player_save_name[i - 1].compare(0, 1, " "))
			infotext += player_save_name[i - 1];
		else {
			std::string temp = player_save_name[i - 1];
			bool firstrun = true;
			while (temp.find(' ', 1) < temp.size()) {
				if (firstrun)
					firstrun = false;
				else
					infotext += "\n    ";
				uint32_t x = temp.find(' ', 1);
				infotext += temp.substr(1, x);
				temp = temp.substr(x + 1, temp.size());
			}
		}
	}
	m_map_info.set_text(infotext);
	m_filename_proof = m_settings->settings().mapfilename;
}

/**
 * load map informations and update the UI
 */
void Fullscreen_Menu_LaunchMPG::load_map_info()
{
	Widelands::Map map; //  Map_Loader needs a place to put it's preload data

	char const * const name = m_settings->settings().mapfilename.c_str();
	std::unique_ptr<Widelands::Map_Loader> ml = map.get_correct_loader(name);
	if (!ml) {
		throw warning(_("There was an error!"), _("The map file seems to be invalid!"));
	}

	map.set_filename(name);
	{
		i18n::Textdomain td("maps");
		ml->preload_map(true);
	}

	// get translated worldsname
	std::string worldpath((format("worlds/%s") % map.get_world_name()).str());
	Profile prof ((worldpath + "/conf").c_str(), nullptr, (format("world_%s") % map.get_world_name()).str());
	Section & global = prof.get_safe_section("world");
	std::string world(global.get_safe_string("name"));

	std::string infotext;
	infotext += std::string(_("Map details:")) + "\n";
	infotext += std::string("• ") + (format(_("Size: %1$u x %2$u"))
					 % map.get_width() % map.get_height()).str() + "\n";
	infotext += std::string("• ") + (format(ngettext("%u Player", "%u Players", m_nr_players))
					 % m_nr_players).str() + "\n";
	infotext += std::string("• ") + (format(_("World: %s")) % world).str() + "\n";
	if (m_settings->settings().scenario)
		infotext += std::string("• ") + (format(_("Scenario mode selected"))).str() + "\n";
	infotext += "\n";
	infotext += map.get_description();
	infotext += "\n";
	infotext += map.get_hint();

	m_map_info.set_text(infotext);
	m_filename_proof = m_settings->settings().mapfilename;
}

/// Show help
void Fullscreen_Menu_LaunchMPG::help_clicked() {
	if (m_help)
		delete m_help;
	m_help = new UI::HelpWindow(this, _("Multiplayer Game Setup"), m_fs);
	m_help->add_paragraph(_("You are in the multiplayer launch game menu."));
	m_help->add_heading(_("Client settings"));
	m_help->add_paragraph
		(_
		 ("On the left side is a list of all clients including you. You can set your role "
		  "With the button following your nickname. Available roles are:"));
	m_help->add_picture_li
		(_
		 ("The player with the color of the flag. If more than one client selected the same color, these "
		  "share control over the player (‘shared kingdom mode’)."),
		 "pics/genstats_enable_plr_08.png");
	m_help->add_picture_li
		(_("Spectator mode, meaning you can see everything, but cannot control any player"),
		"pics/menu_tab_watch.png");
	m_help->add_heading(_("Player settings"));
	m_help->add_paragraph
		(_
		 ("In the middle are the settings for the players. To start a game, each player must be one of the "
		  "following:"));
	m_help->add_picture_li
		(_("Connected to one or more clients (see ‘Client settings’)."), "pics/genstats_nrworkers.png");
	m_help->add_picture_li
		(_
		 ("Connected to a computer player (the face in the picture as well as the mouse hover texts "
		  "indicate the strength of the currently selected computer player)."),
		"pics/ai_Normal.png");
	m_help->add_picture_li(_("Set as shared in starting position for another player."), "pics/shared_in.png");
	m_help->add_picture_li(_("Closed."), "pics/stop.png");
	m_help->add_block
		(_
		 ("The latter three can only be set by the hosting client by left-clicking the ‘type’ button of a "
		  "player. Hosting players can also set the initialization of each player (the set of buildings, "
		  "wares and workers the player starts with) and the tribe and team for computer players"));
	m_help->add_block
		(_
		 ("Every client connected to a player (the set ‘role’ player) can set the tribe and the team "
		  "for that player"));
	m_help->add_heading(_("Map details"));
	m_help->add_paragraph
		(_
		 ("You can see information about the selected map or savegame on the right-hand side. "
		  "A button next to the map name allows the host to change to a different map. "
		  "Furthermore, the host is able to set a specific win condition, and finally "
		  "can start the game as soon as all players are set up."));
}
