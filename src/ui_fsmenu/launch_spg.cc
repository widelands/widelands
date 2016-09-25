/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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
#include "logic/map.h"
#include "logic/map_objects/map_object.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/mapselect.h"
#include "wui/playerdescrgroup.h"

namespace {
static char const* const player_pictures_small[] = {
   "images/players/fsel_editor_set_player_01_pos.png",
   "images/players/fsel_editor_set_player_02_pos.png",
   "images/players/fsel_editor_set_player_03_pos.png",
   "images/players/fsel_editor_set_player_04_pos.png",
   "images/players/fsel_editor_set_player_05_pos.png",
   "images/players/fsel_editor_set_player_06_pos.png",
   "images/players/fsel_editor_set_player_07_pos.png",
   "images/players/fsel_editor_set_player_08_pos.png"};
}  // namespace

FullscreenMenuLaunchSPG::FullscreenMenuLaunchSPG(GameSettingsProvider* const settings,
                                                 GameController* const ctrl,
                                                 bool /* autolaunch */)
   : FullscreenMenuBase(),

     // Values for alignment and size
     butw_(get_w() / 4),
     buth_(get_h() * 9 / 200),

     // Buttons
     select_map_(this,
                 "select_map",
                 get_w() * 7 / 10,
                 get_h() * 3 / 10,
                 butw_,
                 buth_,
                 g_gr->images().get("images/ui_basic/but1.png"),
                 _("Select map"),
                 std::string(),
                 false),
     wincondition_(this,
                   "win_condition",
                   get_w() * 7 / 10,
                   get_h() * 4 / 10 + buth_,
                   butw_,
                   buth_,
                   g_gr->images().get("images/ui_basic/but1.png"),
                   "",
                   std::string(),
                   false),
     back_(this,
           "back",
           get_w() * 7 / 10,
           get_h() * 17 / 20,
           butw_,
           buth_,
           g_gr->images().get("images/ui_basic/but0.png"),
           _("Back")),
     ok_(this,
         "ok",
         get_w() * 7 / 10,
         get_h() * 9 / 10,
         butw_,
         buth_,
         g_gr->images().get("images/ui_basic/but2.png"),
         _("Start game"),
         std::string(),
         false),

     // Text labels
     title_(this, get_w() / 2, get_h() / 10, _("Launch Game"), UI::Align::kHCenter),
     mapname_(this,
              get_w() * 7 / 10 + butw_ / 2,
              get_h() * 53 / 200 - 15,
              std::string(),
              UI::Align::kHCenter),
     name_(this, get_w() * 1 / 25, get_h() * 53 / 200 - 15, _("Player’s name"), UI::Align::kLeft),
     type_(this,
           // (Element x) + (PlayerDescriptionGroup x)  + border
           ((get_w() * 16 / 25) * 35 / 125) + (get_w() / 25) + 2,
           get_h() * 53 / 200 - 15,
           _("Player’s type"),
           UI::Align::kLeft),
     team_(this,
           ((get_w() * 16 / 25) * 35 / 125) + (get_w() / 25) + 2,
           get_h() * 53 / 200,
           _("Team"),
           UI::Align::kLeft),
     tribe_(this,
            ((get_w() * 16 / 25) * 80 / 125) + (get_w() / 25) + 2,
            get_h() * 53 / 200 - 15,
            _("Player’s tribe"),
            UI::Align::kLeft),
     init_(this,
           ((get_w() * 16 / 25) * 55 / 125) + (get_w() / 25) + 2,
           get_h() * 53 / 200,
           _("Start type"),
           UI::Align::kLeft),
     wincondition_type_(this,
                        get_w() * 7 / 10 + (butw_ / 2),
                        get_h() * 7 / 20 + buth_,
                        _("Type of game"),
                        UI::Align::kHCenter),

     // Variables and objects used in the menu
     settings_(settings),
     ctrl_(ctrl),
     is_scenario_(false) {
	select_map_.sigclicked.connect(
	   boost::bind(&FullscreenMenuLaunchSPG::select_map, boost::ref(*this)));
	wincondition_.sigclicked.connect(
	   boost::bind(&FullscreenMenuLaunchSPG::win_condition_clicked, boost::ref(*this)));
	back_.sigclicked.connect(boost::bind(&FullscreenMenuLaunchSPG::clicked_back, boost::ref(*this)));
	ok_.sigclicked.connect(boost::bind(&FullscreenMenuLaunchSPG::clicked_ok, boost::ref(*this)));

	lua_ = new LuaInterface();
	win_condition_scripts_ = settings_->settings().win_condition_scripts;
	cur_wincondition_ = -1;
	win_condition_clicked();

	title_.set_fontsize(UI_FONT_SIZE_BIG);

	int smaller_fontsize = fs_small() * 4 / 5;
	name_.set_fontsize(smaller_fontsize);
	type_.set_fontsize(smaller_fontsize);
	team_.set_fontsize(smaller_fontsize);
	tribe_.set_fontsize(smaller_fontsize);
	init_.set_fontsize(smaller_fontsize);

	uint32_t y = get_h() * 3 / 10 - buth_;
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i) {
		const Image* player_image = g_gr->images().get(player_pictures_small[i]);
		assert(player_image);

		pos_[i] =
		   new UI::Button(this, "switch_to_position", get_w() / 100, y += buth_, get_h() * 17 / 500,
		                  get_h() * 17 / 500, g_gr->images().get("images/ui_basic/but1.png"),
		                  player_image, _("Switch to position"));
		pos_[i]->sigclicked.connect(
		   boost::bind(&FullscreenMenuLaunchSPG::switch_to_position, boost::ref(*this), i));
		players_[i] = new PlayerDescriptionGroup(
		   this, get_w() / 25, y, get_w() * 16 / 25, get_h() * 17 / 500 * 2, settings, i);
		y += buth_ / 1.17;
	}
}

FullscreenMenuLaunchSPG::~FullscreenMenuLaunchSPG() {
	delete lua_;
}

/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void FullscreenMenuLaunchSPG::start() {
	select_map();
	if (settings_->settings().mapname.empty()) {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	}
}

void FullscreenMenuLaunchSPG::think() {
	if (ctrl_)
		ctrl_->think();

	refresh();
}

/**
 * back-button has been pressed
 */
void FullscreenMenuLaunchSPG::clicked_back() {
	//  The following behaviour might look strange at first view, but for the
	//  user it seems as if the launchgame-menu is a child of mapselect and
	//  not the other way around - just end_modal(0); will be seen as bug
	//  from user point of view, so we reopen the mapselect-menu.
	settings_->set_map(std::string(), std::string(), 0);
	select_map();
	if (settings_->settings().mapname.empty())
		return end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	refresh();
}

/**
 * WinCondition button has been pressed
 */
void FullscreenMenuLaunchSPG::win_condition_clicked() {
	if (settings_->can_change_map()) {
		cur_wincondition_++;
		cur_wincondition_ %= win_condition_scripts_.size();
		settings_->set_win_condition_script(win_condition_scripts_[cur_wincondition_]);
	}

	win_condition_update();
}

/**
 * update win conditions information
 */
void FullscreenMenuLaunchSPG::win_condition_update() {
	if (settings_->settings().scenario) {
		wincondition_.set_title(_("Scenario"));
		wincondition_.set_tooltip(_("Win condition is set through the scenario"));
	} else {
		win_condition_load();
	}
}

/**
 * Loads the current win condition script from the settings provider.
 * Calls win_condition_clicked() if the current map can't handle the win condition.
 */
void FullscreenMenuLaunchSPG::win_condition_load() {
	bool is_usable = true;
	try {
		std::unique_ptr<LuaTable> t = lua_->run_script(settings_->get_win_condition_script());
		t->do_not_warn_about_unaccessed_keys();

		// Skip this win condition if the map doesn't have all the required tags
		if (t->has_key("map_tags") && !settings_->settings().mapfilename.empty()) {
			Widelands::Map map;
			std::unique_ptr<Widelands::MapLoader> ml =
			   map.get_correct_loader(settings_->settings().mapfilename);
			ml->preload_map(true);
			for (const std::string& map_tag : t->get_table("map_tags")->array_entries<std::string>()) {
				if (!map.has_tag(map_tag)) {
					is_usable = false;
					break;
				}
			}
		}

		const std::string name = t->get_string("name");
		const std::string descr = t->get_string("description");
		{
			i18n::Textdomain td("win_conditions");
			wincondition_.set_title(_(name));
		}
		wincondition_.set_tooltip(descr.c_str());
	} catch (LuaTableKeyError&) {
		// might be that this is not a win condition after all.
		is_usable = false;
	}
	if (!is_usable) {
		win_condition_clicked();
	}
}

/**
 * start-button has been pressed
 */
void FullscreenMenuLaunchSPG::clicked_ok() {
	if (!g_fs->file_exists(filename_))
		throw WLWarning(_("File not found"),
		                _("Widelands tried to start a game with a file that could not be "
		                  "found at the given path.\n"
		                  "The file was: %s\n"
		                  "If this happens in a network game, the host might have selected "
		                  "a file that you do not own. Normally, such a file should be sent "
		                  "from the host to you, but perhaps the transfer was not yet "
		                  "finished!?!"),
		                filename_.c_str());
	if (settings_->can_launch()) {
		if (is_scenario_) {
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
void FullscreenMenuLaunchSPG::refresh() {
	const GameSettings& settings = settings_->settings();

	{
		// Translate the maps name
		const char* nomap = _("(no map)");
		i18n::Textdomain td("maps");
		mapname_.set_text(settings.mapname.size() != 0 ? _(settings.mapname) : nomap);
	}
	filename_ = settings.mapfilename;
	nr_players_ = settings.players.size();

	ok_.set_enabled(settings_->can_launch());

	select_map_.set_visible(settings_->can_change_map());
	select_map_.set_enabled(settings_->can_change_map());
	wincondition_.set_enabled(settings_->can_change_map() && !settings.scenario);

	if (settings.scenario) {
		set_scenario_values();
	}

	// "Choose Position" Buttons in frond of PDG
	for (uint8_t i = 0; i < nr_players_; ++i) {
		pos_[i]->set_visible(true);
		const PlayerSettings& player = settings.players[i];
		pos_[i]->set_enabled(!is_scenario_ && (player.state == PlayerSettings::stateOpen ||
		                                       player.state == PlayerSettings::stateComputer));
	}
	for (uint32_t i = nr_players_; i < MAX_PLAYERS; ++i)
		pos_[i]->set_visible(false);

	// update the player description groups
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
		players_[i]->refresh();

	win_condition_update();
}

/**
 * Select a map and send all information to the user interface.
 */
void FullscreenMenuLaunchSPG::select_map() {
	if (!settings_->can_change_map())
		return;

	FullscreenMenuMapSelect msm(settings_, nullptr);
	FullscreenMenuBase::MenuTarget code = msm.run<FullscreenMenuBase::MenuTarget>();

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		// Set scenario = false, else the menu might crash when back is pressed.
		settings_->set_scenario(false);
		return;  // back was pressed
	}

	is_scenario_ = code == FullscreenMenuBase::MenuTarget::kScenarioGame;
	settings_->set_scenario(is_scenario_);

	const MapData& mapdata = *msm.get_map();
	nr_players_ = mapdata.nrplayers;

	safe_place_for_host(nr_players_);
	settings_->set_map(mapdata.name, mapdata.filename, nr_players_);
}

/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void FullscreenMenuLaunchSPG::set_scenario_values() {
	if (settings_->settings().mapfilename.empty()) {
		throw wexception("settings()->scenario was set to true, but no map is available");
	}
	Widelands::Map map;  //  MapLoader needs a place to put its preload data
	std::unique_ptr<Widelands::MapLoader> map_loader(
	   map.get_correct_loader(settings_->settings().mapfilename));
	map.set_filename(settings_->settings().mapfilename);
	map_loader->preload_map(true);
	Widelands::PlayerNumber const nrplayers = map.get_nrplayers();
	for (uint8_t i = 0; i < nrplayers; ++i) {
		settings_->set_player_name(i, map.get_scenario_player_name(i + 1));
		settings_->set_player_tribe(i, map.get_scenario_player_tribe(i + 1));
	}
}

/**
 * Called when a position-button was clicked.
 */
void FullscreenMenuLaunchSPG::switch_to_position(uint8_t const pos) {
	settings_->set_player_number(pos);
}

/**
 * Check to avoid segfaults, if the player changes a map with less player
 * positions while being on a later invalid position.
 */
void FullscreenMenuLaunchSPG::safe_place_for_host(uint8_t const newplayernumber) {
	GameSettings settings = settings_->settings();

	// Check whether the host would still keep a valid position and return if
	// yes.
	if (settings.playernum == UserSettings::none() || settings.playernum < newplayernumber)
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
	settings_->set_player_state(0, PlayerSettings::stateClosed);
	settings_->set_player_state(0, PlayerSettings::stateOpen);
	switch_to_position(0);
}
