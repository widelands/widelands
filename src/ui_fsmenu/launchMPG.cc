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
#include "ui_basic/window.h"
#include "warning.h"
#include "wui/gamechatpanel.h"
#include "wui/multiplayersetupgroup.h"

#include "launchMPG.h"

#include <boost/format.hpp>
using boost::format;


/// Simple user interaction window for selecting either map, save or cancel
struct MapOrSaveSelectionWindow : public UI::Window {
	MapOrSaveSelectionWindow
		(UI::Panel * parent, uint32_t w, uint32_t h,
		 uint32_t fontsize, std::string fontname)
	:
	Window(parent, "selection_window", 0, 0, w, h, _("Please select"))
	{
		center_to_parent();

		uint32_t y     = get_inner_h() / 10;
		uint32_t space = get_inner_w() / 40;
		uint32_t butw  = get_inner_w() * 3 / 10;
		uint32_t buth  = get_inner_h() * 8 / 10;
		new UI::Callback_Button
			(this, "map",
			 space, y, butw, buth,
			 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
			 boost::bind
				 (&MapOrSaveSelectionWindow::pressedButton, boost::ref(*this), 1),
			 _("Map"), _("Select a map"), true, false,
			 fontname, fontsize);
		new UI::Callback_Button
			(this, "saved_game",
			 2 * space + butw, y, butw, buth,
			 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
			 boost::bind
				 (&MapOrSaveSelectionWindow::pressedButton, boost::ref(*this), 2),
			 _("Saved game"), _("Select a saved game"), true, false,
			 fontname, fontsize);
		new UI::Callback_Button
			(this, "cancel",
			 3 * space + 2 * butw, y, butw, buth,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 boost::bind
				 (&MapOrSaveSelectionWindow::pressedButton, boost::ref(*this), 0),
			 _("Cancel"), _("Cancel selection"), true, false,
			 fontname, fontsize);
	}

	void pressedButton(uint8_t i) {
		end_modal(i);
	}
	private:
};

Fullscreen_Menu_LaunchMPG::Fullscreen_Menu_LaunchMPG
	(GameSettingsProvider * const settings, GameController * const ctrl)
	:
	Fullscreen_Menu_Base("launchgamemenu.jpg"),

// Values for alignment and size
	m_butw (m_xres / 4),
	m_buth (m_yres * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Buttons
	m_change_map_or_save
		(this, "change_map_or_save",
		 m_xres * 37 / 50 + m_butw - m_buth, m_yres * 3 / 20, m_buth, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_UI, "pics/menu_toggle_minimap.png"),
		 boost::bind
			 (&Fullscreen_Menu_LaunchMPG::change_map_or_save, boost::ref(*this)),
		 _("Change map or saved game"), false, false,
		 m_fn, m_fs),
	m_ok
		(this, "ok",
		 m_xres * 37 / 50, m_yres * 12 / 20, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
		 boost::bind
			 (&Fullscreen_Menu_LaunchMPG::start_clicked, boost::ref(*this)),
		 _("Start game"), std::string(), false, false,
		 m_fn, m_fs),
	m_back
		(this, "back",
		 m_xres * 37 / 50, m_yres * 218 / 240, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 boost::bind(&Fullscreen_Menu_LaunchMPG::back_clicked, boost::ref(*this)),
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs),
	m_wincondition
		(this, "win_condition",
		 m_xres * 37 / 50, m_yres * 11 / 20, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 boost::bind
			 (&Fullscreen_Menu_LaunchMPG::win_condition_clicked,
			  boost::ref(*this)),
		 "", std::string(), false, false,
		 m_fn, m_fs),

// Text labels
	m_title
		(this,
		 m_xres / 2, m_yres / 18,
		 _("Multiplayer Game Setup"), UI::Align_HCenter),
	m_mapname
		(this,
		 m_xres * 37 / 50, m_yres * 3 / 20,
		 std::string()),

	m_map_info
		(this, m_xres * 37 / 50, m_yres * 2 / 10, m_butw, m_yres * 27 / 80),

	m_client_info
		(this, m_xres * 37 / 50, m_yres * 7 / 10, m_butw, m_yres * 5 / 20),

// Variables and objects used in the menu
	m_settings     (settings),
	m_ctrl         (ctrl),
	m_chat         (0)
{
	// Register win condition scripts
	m_lua = create_LuaInterface();
	m_lua->register_scripts(*g_fs, "win_conditions", "scripting/win_conditions");

	ScriptContainer sc = m_lua->get_scripts_for("win_conditions");
	container_iterate_const(ScriptContainer, sc, wc)
		m_win_conditions.push_back(wc->first);
	m_cur_wincondition = -1;
	win_condition_clicked();

	m_title      .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_mapname    .set_font(m_fn, m_fs, RGBColor(255, 255, 127));
	m_client_info.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_map_info   .set_font(m_fn, m_fs, UI_FONT_CLR_FG);

	m_mapname .set_text(_("(no map)"));
	m_map_info.set_text(_("The host has not yet selected a map or saved game."));

	m_mpsg =
		new MultiPlayerSetupGroup
			(this,
			 m_xres / 50, m_yres / 10, m_xres * 57 / 80, m_yres * 21 / 40,
			 settings, m_butw, m_buth, m_fn, m_fs);

	// If we are the host, open the map or save selection menu at startup
	if (m_settings->settings().usernum == 0
		 && m_settings->settings().mapname.empty())
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
			 m_xres / 50, m_yres * 13 / 20, m_xres * 57 / 80, m_yres * 3 / 10,
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
	} else if (m_settings->settings().savegame) {
		m_wincondition.set_title(_("Savegame"));
		m_wincondition.set_tooltip
			(_("The game is a saved game - the win condition was set before."));
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

/// Opens a popup window to select a map or saved game 
void Fullscreen_Menu_LaunchMPG::change_map_or_save() {
	MapOrSaveSelectionWindow selection_window
		(this, m_xres / 2, m_yres / 20, m_fs, m_fn);
	switch(selection_window.run()) {
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

	GameSettings const & settings = m_settings->settings();

	Fullscreen_Menu_MapSelect msm
		(settings.multiplayer ? Map::MP_SCENARIO : Map::SP_SCENARIO);
	int code = msm.run();

	if (code <= 0) {
		// Set scenario = false, else the menu might crash when back is pressed.
		m_settings->setScenario(false);
		return;  // back was pressed
	}

	m_settings->setScenario(code == 2);

	MapData const & mapdata = *msm.get_map();
	m_nr_players = mapdata.nrplayers;

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
	Fullscreen_Menu_LoadGame lsgm(game);
	int code = lsgm.run();

	if (code <= 0)
		return; // back was pressed

	std::string filename = lsgm.filename();

	// Read the needed data from file "elemental" of the used map.
	FileSystem & l_fs = g_fs->MakeSubFileSystem(filename.c_str());
	Profile prof;
	prof.read("map/elemental", 0, l_fs);
	Section & s = prof.get_safe_section("global");

	std::string mapname = s.get_safe_string("name");
	m_nr_players = s.get_safe_int("nr_players");

	m_settings->setMap(mapname, filename, m_nr_players, true);
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
			 	 "found at given path.\n"
			 	 "The file was: %s\n"
			 	 "If this happens, the host might have selected a file that you do "
			 	 "not own. Normally such a file should be send from the host to "
			 	 "you, but perhaps the transfer was not yet finnished!?!"),
			 m_settings->settings().mapfilename.c_str());
	if (m_settings->canLaunch()) {
		GameSettings const & s = m_settings->settings();
		if (s.savegame)
			end_modal(1 + s.scenario);
		else
			end_modal(3 + s.scenario);
	}
}


/**
 * update the user interface and take care about the visibility of
 * buttons and text.
 */
void Fullscreen_Menu_LaunchMPG::refresh()
{
	GameSettings const & settings = m_settings->settings();

	if (settings.mapfilename != m_filename_proof) {
		if(!g_fs->FileExists(settings.mapfilename)) {
			m_client_info.set_font(m_fn, m_fs, UI_FONT_CLR_WARNING);
			m_client_info.set_text
				(_("The selected file can not be found. If it is not automatically "
				   "transfered to you, please write the host about this problem."));
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
			else
				load_map_info();
			m_mapname.set_text(settings.mapname);
		}
	}

	m_ok.set_enabled(m_settings->canLaunch());

	m_change_map_or_save.set_enabled(m_settings->canChangeMap());
	m_change_map_or_save.set_visible(m_settings->canChangeMap());

	m_wincondition.set_enabled
		(m_settings->canChangeMap() && !settings.savegame && !settings.scenario);

	if (settings.scenario)
		set_scenario_values();

	win_condition_update();
	m_mpsg->refresh();
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
		m_settings->setPlayerTribe(i, map.get_scenario_player_tribe(i + 1));
	}
}

/**
 * load all playerdata from savegame and update UI accordingly
 * TODO set current clients accordingly
 */
void Fullscreen_Menu_LaunchMPG::load_previous_playerdata()
{
	FileSystem & l_fs = g_fs->MakeSubFileSystem
		(m_settings->settings().mapfilename.c_str());
	Profile prof;
	prof.read("map/player_names", 0, l_fs);
	std::string strbuf;
	std::string infotext = _("Saved players are:");
	std::string player_save_name[MAX_PLAYERS];
	std::string player_save_tribe[MAX_PLAYERS];
	char buf[32];

	uint8_t i = 1;
	for (; i <= m_nr_players; ++i) {
		infotext += "\n* ";
		strbuf = std::string();
		snprintf(buf, sizeof(buf), "player_%u", i);
		Section & s = prof.get_safe_section(buf);
		player_save_name [i - 1] = s.get_string("name");
		player_save_tribe[i - 1] = s.get_string("tribe");

		snprintf(buf, sizeof(buf), "Player %u", i);
		infotext += buf;
		if (player_save_tribe[i - 1].empty()) {
			infotext += ":\n    ";
			infotext += _("<closed>");
			continue; // if tribe is empty, the player does not exist
		}

		// get translated tribename
		strbuf = "tribes/" + player_save_tribe[i - 1];
		strbuf += "/conf";
		Profile tribe(strbuf.c_str(), 0, "tribe_" + player_save_tribe[i - 1]);
		Section & global = tribe.get_safe_section("tribe");
		player_save_tribe[i - 1] = global.get_safe_string("name");
		infotext += " (";
		infotext += player_save_tribe[i - 1];
		infotext += "):\n    ";
		infotext += player_save_name[i - 1];
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
	i18n::Textdomain td("maps");

	char const * const name = m_settings->settings().mapfilename.c_str();
	Widelands::Map_Loader * const ml = map.get_correct_loader(name);
	if (!ml)
		throw warning
			(_("There was an error!"), _("The map file seems to be invalid!"));

	map.set_filename(name);
	ml->preload_map(true);
	delete ml;

	std::string infotext = _("Map informations:\n");
	infotext +=
		(format(_("* Size: %ux%u\n")) % map.get_width() % map.get_height()).str();
	infotext += (format(_("* %i Players\n")) % m_nr_players).str();

	// get translated worldsname
	std::string worldpath((format("worlds/%s") % map.get_world_name()).str());
	Profile prof
		((worldpath + "/conf").c_str(), 0,
		 (format("world_%s") % map.get_world_name()).str());
	Section & global = prof.get_safe_section("world");
	std::string world(global.get_safe_string("name"));
	infotext += (format(_("* World type: %s\n")) % world).str();

	if (m_settings->settings().scenario)
		infotext += (format(_("* Scenario mode selected\n"))).str();
	infotext += "\n";
	infotext += map.get_description();

	m_map_info.set_text(infotext);
	m_filename_proof = m_settings->settings().mapfilename;
}
