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

#include "ui_fsmenu/launch_spg.h"

#include <memory>

#include <boost/algorithm/string/predicate.hpp>

#include "base/i18n.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "helper.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/constants.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/instances.h"
#include "logic/map.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/mapselect.h"
#include "wui/playerdescrgroup.h"

FullscreenMenuLaunchSPG::FullscreenMenuLaunchSPG
	(GameSettingsProvider * const settings, GameController * const ctrl,
	 bool /* autolaunch */)
	:
	FullscreenMenuBase("ui_fsmenu.jpg"),

// Values for alignment and size
	m_butw (get_w() / 4),
	m_buth (get_h() * 9 / 200),

// Buttons
	m_select_map
		(this, "select_map",
		 get_w() * 7 / 10, get_h() * 3 / 10, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("Select map"), std::string(), false, false),
	m_wincondition
		(this, "win_condition",
		 get_w() * 7 / 10, get_h() * 4 / 10 + m_buth, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 "", std::string(), false, false),
	m_back
		(this, "back",
		 get_w() * 7 / 10, get_h() * 17 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),
	m_ok
		(this, "ok",
		 get_w() * 7 / 10, get_h() * 9 / 10, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("Start game"), std::string(), false, false),

// Text labels
	m_title
		(this,
		 get_w() / 2, get_h() / 10,
		 _("Launch Game"), UI::Align_HCenter),
	m_mapname
		(this,
		 get_w() * 7 / 10 + m_butw / 2, get_h() * 53 / 200 - 15,
		 std::string(), UI::Align_HCenter),
	m_name
		(this,
		 get_w() * 1 / 25, get_h() * 53 / 200 - 15,
		 _("Player’s name"), UI::Align_Left),
	m_type
		(this,
		 // (Element x) + (PlayerDescriptionGroup x)  + border
		 ((get_w() * 16 / 25) * 35 / 125) + (get_w() / 25) + 2, get_h() * 53 / 200 - 15,
		 _("Player’s type"), UI::Align_Left),
	m_team
		(this,
		 ((get_w() * 16 / 25) * 35 / 125) + (get_w() / 25) + 2, get_h() * 53 / 200,
		 _("Team"), UI::Align_Left),
	m_tribe
		(this,
		 ((get_w() * 16 / 25) * 80 / 125) + (get_w() / 25) + 2, get_h() * 53 / 200 - 15,
		 _("Player’s tribe"), UI::Align_Left),
	m_init
		(this,
		 ((get_w() * 16 / 25) * 55 / 125) + (get_w() / 25) + 2, get_h() * 53 / 200,
		 _("Start type"), UI::Align_Left),
	m_wincondition_type
		(this,
		 get_w() * 7 / 10 + (m_butw / 2), get_h() * 7 / 20 + m_buth,
		 _("Type of game"), UI::Align_HCenter),

// Variables and objects used in the menu
	m_settings     (settings),
	m_ctrl         (ctrl),
	m_is_scenario  (false)
{
	m_select_map.sigclicked.connect(boost::bind(&FullscreenMenuLaunchSPG::select_map, boost::ref(*this)));
	m_wincondition.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuLaunchSPG::win_condition_clicked,
			  boost::ref(*this)));
	m_back.sigclicked.connect(boost::bind(&FullscreenMenuLaunchSPG::clicked_back, boost::ref(*this)));
	m_ok.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuLaunchSPG::clicked_ok, boost::ref(*this)));

	m_lua = new LuaInterface();
	m_win_condition_scripts = m_settings->settings().win_condition_scripts;
	m_cur_wincondition = -1;
	win_condition_clicked();

	m_title  .set_textstyle(UI::TextStyle::ui_big());
	m_mapname.set_textstyle(UI::TextStyle::ui_small());
	m_wincondition_type.set_textstyle(UI::TextStyle::ui_small());

	UI::TextStyle tsmaller
		(UI::TextStyle::makebold
		 (UI::Font::get(ui_fn(), fs_small() * 4 / 5), UI_FONT_CLR_FG));
	m_name.set_textstyle(tsmaller);
	m_type.set_textstyle(tsmaller);
	m_team.set_textstyle(tsmaller);
	m_tribe.set_textstyle(tsmaller);
	m_init.set_textstyle(tsmaller);

	uint32_t y = get_h() * 3 / 10 - m_buth;
	char posIco[42];
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i) {
		sprintf(posIco, "pics/fsel_editor_set_player_0%i_pos.png", i + 1);
		m_pos[i] =
			new UI::Button
				(this, "switch_to_position",
				 get_w() / 100, y += m_buth, get_h() * 17 / 500, get_h() * 17 / 500,
				 g_gr->images().get("pics/but1.png"),
				 g_gr->images().get(posIco),
				 _("Switch to position"), false);
		m_pos[i]->sigclicked.connect
			(boost::bind(&FullscreenMenuLaunchSPG::switch_to_position, boost::ref(*this), i));
		m_players[i] =
			new PlayerDescriptionGroup
				(this,
				 get_w() / 25, y, get_w() * 16 / 25, get_h() * 17 / 500 * 2,
				 settings, i);
		y += m_buth / 1.17;
	}
}

FullscreenMenuLaunchSPG::~FullscreenMenuLaunchSPG() {
	delete m_lua;
}

/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void FullscreenMenuLaunchSPG::start()
{
	select_map();
	if (m_settings->settings().mapname.empty()) {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	}
}


void FullscreenMenuLaunchSPG::think()
{
	if (m_ctrl)
		m_ctrl->think();

	refresh();
}


/**
 * back-button has been pressed
 */
void FullscreenMenuLaunchSPG::clicked_back()
{
	//  The following behaviour might look strange at first view, but for the
	//  user it seems as if the launchgame-menu is a child of mapselect and
	//  not the other way around - just end_modal(0); will be seen as bug
	//  from user point of view, so we reopen the mapselect-menu.
	m_settings->set_map(std::string(), std::string(), 0);
	select_map();
	if (m_settings->settings().mapname.empty())
		return end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	refresh();
}

/**
 * WinCondition button has been pressed
 */
void FullscreenMenuLaunchSPG::win_condition_clicked()
{
	if (m_settings->can_change_map()) {
		m_cur_wincondition++;
		m_cur_wincondition %= m_win_condition_scripts.size();
		m_settings->set_win_condition_script(m_win_condition_scripts[m_cur_wincondition]);
	}

	win_condition_update();
}

/**
 * update win conditions information
 */
void FullscreenMenuLaunchSPG::win_condition_update() {
	if (m_settings->settings().scenario) {
		m_wincondition.set_title(_("Scenario"));
		m_wincondition.set_tooltip
			(_("Win condition is set through the scenario"));
	} else {
		try {
			std::unique_ptr<LuaTable> t =
			   m_lua->run_script(m_settings->get_win_condition_script());
			t->do_not_warn_about_unaccessed_keys();
			const std::string name = t->get_string("name");
			const std::string descr = t->get_string("description");
			{
				i18n::Textdomain td("win_conditions");
				m_wincondition.set_title(_(name));
			}
			m_wincondition.set_tooltip(descr.c_str());
		} catch (LuaTableKeyError &) {
			// might be that this is not a win condition after all.
			win_condition_clicked();
		}
	}
}

/**
 * start-button has been pressed
 */
void FullscreenMenuLaunchSPG::clicked_ok()
{
	if (!g_fs->file_exists(m_filename))
		throw WLWarning
			(_("File not found"),
			 _
			 	("Widelands tried to start a game with a file that could not be "
			 	 "found at the given path.\n"
			 	 "The file was: %s\n"
			 	 "If this happens in a network game, the host might have selected "
			 	 "a file that you do not own. Normally, such a file should be sent "
			 	 "from the host to you, but perhaps the transfer was not yet "
			 	 "finished!?!"),
			 m_filename.c_str());
	if (m_settings->can_launch()) {
		if (m_is_scenario) {
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kScenarioGame);
		} else {
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kNormalGame);
		}
	}
}


/**
 * update the user interface and take care about the visibility of
 * buttons and text.
 */
void FullscreenMenuLaunchSPG::refresh()
{
	const GameSettings & settings = m_settings->settings();

	{
		// Translate the maps name
		const char * nomap = _("(no map)");
		i18n::Textdomain td("maps");
		m_mapname.set_text(settings.mapname.size() != 0 ? _(settings.mapname) : nomap);
	}
	m_filename = settings.mapfilename;
	m_nr_players = settings.players.size();

	m_ok.set_enabled(m_settings->can_launch());

	m_select_map.set_visible(m_settings->can_change_map());
	m_select_map.set_enabled(m_settings->can_change_map());
	m_wincondition.set_enabled
		(m_settings->can_change_map() && !settings.scenario);

	if (settings.scenario)
		set_scenario_values();

	// "Choose Position" Buttons in frond of PDG
	for (uint8_t i = 0; i < m_nr_players; ++i) {
		m_pos[i]->set_visible(true);
		const PlayerSettings & player = settings.players[i];
		if
			(player.state == PlayerSettings::stateOpen ||
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
void FullscreenMenuLaunchSPG::select_map()
{
	if (!m_settings->can_change_map())
		return;

	FullscreenMenuMapSelect msm(m_settings, nullptr);
	FullscreenMenuBase::MenuTarget code = msm.run<FullscreenMenuBase::MenuTarget>();

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		// Set scenario = false, else the menu might crash when back is pressed.
		m_settings->set_scenario(false);
		return;  // back was pressed
	}

	m_is_scenario = code == FullscreenMenuBase::MenuTarget::kScenarioGame;
	m_settings->set_scenario(m_is_scenario);

	const MapData & mapdata = *msm.get_map();
	m_nr_players = mapdata.nrplayers;

	safe_place_for_host(m_nr_players);
	m_settings->set_map(mapdata.name, mapdata.filename, m_nr_players);
}


/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void FullscreenMenuLaunchSPG::set_scenario_values()
{
	if (m_settings->settings().mapfilename.empty())
		throw wexception
				("settings()->scenario was set to true, but no map is available");
	Widelands::Map map; //  MapLoader needs a place to put its preload data
	std::unique_ptr<Widelands::MapLoader> map_loader(
	   map.get_correct_loader(m_settings->settings().mapfilename));
	map.set_filename(m_settings->settings().mapfilename);
	map_loader->preload_map(true);
	Widelands::PlayerNumber const nrplayers = map.get_nrplayers();
	for (uint8_t i = 0; i < nrplayers; ++i) {
		m_settings->set_player_name (i, map.get_scenario_player_name (i + 1));
		m_settings->set_player_tribe(i, map.get_scenario_player_tribe(i + 1));
	}
}

/**
 * Called when a position-button was clicked.
 */
void FullscreenMenuLaunchSPG::switch_to_position(uint8_t const pos)
{
	m_settings->set_player_number(pos);
}


/**
 * Check to avoid segfaults, if the player changes a map with less player
 * positions while being on a later invalid position.
 */
void FullscreenMenuLaunchSPG::safe_place_for_host
	(uint8_t const newplayernumber)
{
	GameSettings settings = m_settings->settings();

	// Check whether the host would still keep a valid position and return if
	// yes.
	if
		(settings.playernum == UserSettings::none() ||
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
	m_settings->set_player_state(0, PlayerSettings::stateClosed);
	m_settings->set_player_state(0, PlayerSettings::stateOpen);
	switch_to_position(0);
}
