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

#include "ui_fsmenu/launch_mpg.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/warning.h"
#include "graphic/graphic.h"
#include "graphic/playercolor.h"
#include "graphic/text_constants.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/map.h"
#include "logic/map_objects/map_object.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/mapselect.h"
#include "wui/gamechatpanel.h"
#include "wui/multiplayersetupgroup.h"

/// Simple user interaction window for selecting either map, save or cancel
struct MapOrSaveSelectionWindow : public UI::Window {
	MapOrSaveSelectionWindow(UI::Panel* parent, GameController* gc, uint32_t w, uint32_t h)
	   : /** TRANSLATORS: Dialog box title for selecting between map or saved game for new
	        multiplayer game */
	     Window(parent, "selection_window", 0, 0, w, h, _("Please select")),
	     ctrl_(gc) {
		center_to_parent();

		uint32_t y = get_inner_h() / 10;
		uint32_t space = y;
		uint32_t butw = get_inner_w() - 2 * space;
		uint32_t buth = (get_inner_h() - 2 * space) / 5;
		UI::Button* btn = new UI::Button(this, "map", space, y, butw, buth,
		                                 g_gr->images().get("images/ui_basic/but0.png"), _("Map"),
		                                 _("Select a map"));
		btn->sigclicked.connect(boost::bind(&MapOrSaveSelectionWindow::pressedButton,
		                                    boost::ref(*this),
		                                    FullscreenMenuBase::MenuTarget::kNormalGame));

		btn = new UI::Button(this, "saved_game", space, y + buth + space, butw, buth,
		                     g_gr->images().get("images/ui_basic/but0.png"),
		                     /** Translators: This is a button to select a savegame */
		                     _("Saved Game"), _("Select a saved game"));
		btn->sigclicked.connect(boost::bind(&MapOrSaveSelectionWindow::pressedButton,
		                                    boost::ref(*this),
		                                    FullscreenMenuBase::MenuTarget::kScenarioGame));

		btn = new UI::Button(this, "cancel", space + butw / 4, y + 3 * buth + 2 * space, butw / 2,
		                     buth, g_gr->images().get("images/ui_basic/but1.png"), _("Cancel"),
		                     _("Cancel selection"));
		btn->sigclicked.connect(boost::bind(&MapOrSaveSelectionWindow::pressedButton,
		                                    boost::ref(*this),
		                                    FullscreenMenuBase::MenuTarget::kBack));
	}

	void think() override {
		if (ctrl_)
			ctrl_->think();
	}

	void pressedButton(FullscreenMenuBase::MenuTarget i) {
		end_modal<FullscreenMenuBase::MenuTarget>(i);
	}

private:
	GameController* ctrl_;
};

FullscreenMenuLaunchMPG::FullscreenMenuLaunchMPG(GameSettingsProvider* const settings,
                                                 GameController* const ctrl)
   : FullscreenMenuBase(),

     // Values for alignment and size
     butw_(get_w() / 4),
     buth_(get_h() * 9 / 200),
     fs_(fs_small()),
     // TODO(GunChleoc): We still need to use these consistently. Just getting them in for now
     // so we can have the SuggestedTeamsBox
     padding_(4),
     indent_(10),
     label_height_(20),
     right_column_x_(get_w() * 37 / 50),

     // Buttons
     change_map_or_save_(this,
                         "change_map_or_save",
                         right_column_x_ + butw_ - buth_,
                         get_h() * 3 / 20,
                         buth_,
                         buth_,
                         g_gr->images().get("images/ui_basic/but1.png"),
                         g_gr->images().get("images/wui/menus/menu_toggle_minimap.png"),
                         _("Change map or saved game")),
     win_condition_dropdown_(this,
                             right_column_x_,
                             get_h() * 11 / 20 - 2 * label_height_,
                             butw_,
                             get_h() - get_h() * 4 / 10 - buth_,
                             ""),
     ok_(this,
         "ok",
         right_column_x_,
         get_h() * 12 / 20 - 2 * label_height_,
         butw_,
         buth_,
         g_gr->images().get("images/ui_basic/but2.png"),
         _("Start game")),
     back_(this,
           "back",
           right_column_x_,
           get_h() * 218 / 240,
           butw_,
           buth_,
           g_gr->images().get("images/ui_basic/but0.png"),
           _("Back")),
     help_button_(this,
                  "help",
                  right_column_x_ + butw_ - buth_,
                  get_h() / 100,
                  buth_,
                  buth_,
                  g_gr->images().get("images/ui_basic/but1.png"),
                  g_gr->images().get("images/ui_basic/menu_help.png"),
                  _("Show the help window")),

     // Text labels
     title_(this, get_w() / 2, get_h() / 25, _("Multiplayer Game Setup"), UI::Align::kHCenter),
     mapname_(this, right_column_x_, get_h() * 3 / 20, std::string()),
     clients_(this,
              // (get_w() * 57 / 80) is the width of the MultiPlayerSetupGroup
              get_w() / 50,
              get_h() / 10,
              (get_w() * 57 / 80) / 3,
              get_h() / 10,
              _("Clients"),
              UI::Align::kHCenter),
     players_(this,
              get_w() / 50 + (get_w() * 57 / 80) * 6 / 15,
              get_h() / 10,
              (get_w() * 57 / 80) * 9 / 15,
              get_h() / 10,
              _("Players"),
              UI::Align::kHCenter),
     map_(this, right_column_x_, get_h() / 10, butw_, get_h() / 10, _("Map"), UI::Align::kHCenter),
     wincondition_type_(this,
                        right_column_x_ + (butw_ / 2),
                        get_h() * 10 / 20 - 1.5 * label_height_,
                        _("Type of game"),
                        UI::Align::kHCenter),

     map_info_(
        this, right_column_x_, get_h() * 2 / 10, butw_, get_h() * 23 / 80 - 2 * label_height_),
     client_info_(
        this, right_column_x_, get_h() * 13 / 20 - 2 * label_height_, butw_, 2 * label_height_),
     help_(nullptr),

     // Variables and objects used in the menu
     settings_(settings),
     ctrl_(ctrl),
     chat_(nullptr) {
	change_map_or_save_.sigclicked.connect(
	   boost::bind(&FullscreenMenuLaunchMPG::change_map_or_save, boost::ref(*this)));
	win_condition_dropdown_.selected.connect(
	   boost::bind(&FullscreenMenuLaunchMPG::win_condition_selected, this));
	ok_.sigclicked.connect(boost::bind(&FullscreenMenuLaunchMPG::clicked_ok, boost::ref(*this)));
	back_.sigclicked.connect(boost::bind(&FullscreenMenuLaunchMPG::clicked_back, boost::ref(*this)));
	help_button_.sigclicked.connect(
	   boost::bind(&FullscreenMenuLaunchMPG::help_clicked, boost::ref(*this)));

	lua_ = new LuaInterface();
	// NOCOM win_condition_clicked();

	title_.set_fontsize(fs_big());
	mapname_.set_fontsize(fs_);
	mapname_.set_color(RGBColor(255, 255, 127));
	clients_.set_fontsize(fs_);
	clients_.set_color(RGBColor(0, 255, 0));
	players_.set_fontsize(fs_);
	players_.set_color(RGBColor(0, 255, 0));
	map_.set_fontsize(fs_);
	map_.set_color(RGBColor(0, 255, 0));

	mapname_.set_text(_("(no map)"));
	map_info_.set_text(_("The host has not yet selected a map or saved game."));

	mpsg_ = new MultiPlayerSetupGroup(
	   this, get_w() / 50, get_h() / 8, get_w() * 57 / 80, get_h(), settings, butw_, buth_);

	// If we are the host, open the map or save selection menu at startup
	if (settings_->settings().usernum == 0 && settings_->settings().mapname.empty()) {
		change_map_or_save();
		// Try to associate the host with the first player
		if (!settings_->settings().players.empty()) {
			settings_->set_player_number(0);
		}
	}

	// Y coordinate will be set later, when we know how high this box will get.
	suggested_teams_box_ =
	   new UI::SuggestedTeamsBox(this, right_column_x_, 0, UI::Box::Vertical, padding_, indent_,
	                             get_w() - right_column_x_, 4 * label_height_);
}

FullscreenMenuLaunchMPG::~FullscreenMenuLaunchMPG() {
	delete lua_;
	delete mpsg_;
	delete chat_;
}

void FullscreenMenuLaunchMPG::think() {
	if (ctrl_)
		ctrl_->think();

	refresh();
}

/**
 * Set a new chat provider.
 *
 * This automatically creates and displays a chat panel when appropriate.
 */
void FullscreenMenuLaunchMPG::set_chat_provider(ChatProvider& chat) {
	delete chat_;
	chat_ = new GameChatPanel(
	   this, get_w() / 50, get_h() * 13 / 20, get_w() * 57 / 80, get_h() * 3 / 10, chat);
}

/**
 * back-button has been pressed
 */
void FullscreenMenuLaunchMPG::clicked_back() {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
}

/**
 * Fill the dropdown with the available win conditions.
 */
void FullscreenMenuLaunchMPG::update_win_conditions() {
	win_condition_dropdown_.clear();
	win_condition_dropdown_.set_enabled(settings_->can_change_map() &&
	                                    !settings_->settings().savegame &&
	                                    !settings_->settings().scenario);  // NOCOM new
	if (settings_->settings().scenario) {
		win_condition_dropdown_.set_label(_("Scenario"));
		win_condition_dropdown_.set_tooltip(_("Win condition is set through the scenario"));
	} else if (settings_->settings().savegame) {  // NOCOM new
		/** Translators: This is a game type */
		win_condition_dropdown_.set_label(_("Saved Game"));
		win_condition_dropdown_.set_tooltip(
		   _("The game is a saved game – the win condition was set before."));
	} else {
		win_condition_dropdown_.set_label("");
		win_condition_dropdown_.set_tooltip("");
		Widelands::Map map;
		std::unique_ptr<Widelands::MapLoader> ml =
		   map.get_correct_loader(settings_->settings().mapfilename);
		if (ml != nullptr) {
			ml->preload_map(true);
			load_win_conditions(map);
		} else {
			const std::string error_message =
			   (boost::format(_("Unable to determine valid win conditions because the map '%s' could "
			                    "not be loaded.")) %
			    settings_->settings().mapfilename)
			      .str();
			win_condition_dropdown_.set_label(_("Error"));
			win_condition_dropdown_.set_tooltip(error_message);
			log("LaunchMPG: No map loader: %s\n", error_message.c_str());
		}
		if (settings_->can_change_map()) {
			win_condition_selected();  // NOCOM new
		}
	}
}

void FullscreenMenuLaunchMPG::load_win_conditions(const Widelands::Map& map) {
	try {
		const std::set<std::string> tags = map.get_tags();
		// Make sure that the last win condition is still valid. If not, pick the first one
		// available.
		if (last_win_condition_.empty()) {
			last_win_condition_ = settings_->settings().win_condition_scripts.front();
		}
		std::unique_ptr<LuaTable> t = win_condition_if_valid(last_win_condition_, tags);
		for (const std::string& win_condition_script : settings_->settings().win_condition_scripts) {
			if (t) {
				break;
			} else {
				last_win_condition_ = win_condition_script;
				t = win_condition_if_valid(last_win_condition_, tags);
			}
		}

		// Now fill the dropdown.
		for (const std::string& win_condition_script : settings_->settings().win_condition_scripts) {
			try {
				t = win_condition_if_valid(win_condition_script, tags);
				if (t) {
					i18n::Textdomain td("win_conditions");
					win_condition_dropdown_.add(_(t->get_string("name")), win_condition_script, nullptr,
					                            win_condition_script == last_win_condition_,
					                            t->get_string("description"));
				}
			} catch (LuaTableKeyError& e) {
				log("LaunchSPG: Error loading win condition: %s %s\n", win_condition_script.c_str(),
				    e.what());
			}
		}
	} catch (const std::exception& e) {
		const std::string error_message =
		   (boost::format(_("Unable to determine valid win conditions because the map '%s' "
		                    "could not be loaded.")) %
		    settings_->settings().mapfilename)
		      .str();
		win_condition_dropdown_.set_label(_("Error"));
		win_condition_dropdown_.set_tooltip(error_message);
		log("LaunchSPG: Exception: %s %s\n", error_message.c_str(), e.what());
	}
}

void FullscreenMenuLaunchMPG::win_condition_selected() {
	last_win_condition_ = win_condition_dropdown_.get_selected();
	if (settings_->can_change_map()) {
		settings_->set_win_condition_script(win_condition_dropdown_.get_selected());  // NOCOM new
	}
}

// TODO(GunChleoc): Turn this into a free standing function. It seems it is not using any state.
std::unique_ptr<LuaTable>
FullscreenMenuLaunchMPG::win_condition_if_valid(const std::string& win_condition_script,
                                                std::set<std::string> tags) const {
	bool is_usable = true;
	std::unique_ptr<LuaTable> t;
	try {
		t = lua_->run_script(win_condition_script);
		t->do_not_warn_about_unaccessed_keys();

		// Skip this win condition if the map doesn't have all the required tags
		if (t->has_key("map_tags")) {
			for (const std::string& map_tag : t->get_table("map_tags")->array_entries<std::string>()) {
				if (!tags.count(map_tag)) {
					is_usable = false;
					break;
				}
			}
		}
	} catch (LuaTableKeyError& e) {
		log(
		   "LaunchSPG: Error loading win condition: %s %s\n", win_condition_script.c_str(), e.what());
	}
	if (!is_usable) {
		t.reset(nullptr);
	}
	return t;
}

/// Opens a popup window to select a map or saved game
void FullscreenMenuLaunchMPG::change_map_or_save() {
	MapOrSaveSelectionWindow selection_window(this, ctrl_, get_w() / 3, get_h() / 4);
	auto result = selection_window.run<FullscreenMenuBase::MenuTarget>();
	assert(result == FullscreenMenuBase::MenuTarget::kNormalGame ||
	       result == FullscreenMenuBase::MenuTarget::kScenarioGame ||
	       result == FullscreenMenuBase::MenuTarget::kBack);
	if (result == FullscreenMenuBase::MenuTarget::kNormalGame) {
		select_map();
	} else if (result == FullscreenMenuBase::MenuTarget::kScenarioGame) {
		select_saved_game();
	}
	update_win_conditions();  // NOCOM new
	                          // NOCOM get the client in line!
}

/**
 * Select a map and send all information to the user interface.
 */
void FullscreenMenuLaunchMPG::select_map() {
	if (!settings_->can_change_map())
		return;

	FullscreenMenuMapSelect msm(settings_, ctrl_);
	FullscreenMenuBase::MenuTarget code = msm.run<FullscreenMenuBase::MenuTarget>();

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		// Set scenario = false, else the menu might crash when back is pressed.
		settings_->set_scenario(false);
		return;
	}

	settings_->set_scenario(code == FullscreenMenuBase::MenuTarget::kScenarioGame);

	const MapData& mapdata = *msm.get_map();
	nr_players_ = mapdata.nrplayers;

	// If the same map was selected again, maybe the state of the "scenario" check box was changed
	// So we should recheck all map predefined values,
	// which is done in refresh(), if filename_proof_ is different to settings.mapfilename -> dummy
	// rename
	if (mapdata.filename == filename_proof_)
		filename_proof_ = filename_proof_ + "new";

	settings_->set_map(mapdata.name, mapdata.filename, nr_players_);
}

/**
 * Select a multi player saved game and send all information to the user
 * interface.
 */
void FullscreenMenuLaunchMPG::select_saved_game() {
	if (!settings_->can_change_map())
		return;

	Widelands::Game game;  // The place all data is saved to.
	FullscreenMenuLoadGame lsgm(game, settings_, ctrl_);
	FullscreenMenuBase::MenuTarget code = lsgm.run<FullscreenMenuBase::MenuTarget>();

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		return;  // back was pressed
	}

	// Saved game was selected - therefore not a scenario
	settings_->set_scenario(false);

	std::string filename = lsgm.filename();

	if (g_fs->file_exists(filename.c_str())) {
		// Read the needed data from file "elemental" of the used map.
		std::unique_ptr<FileSystem> l_fs(g_fs->make_sub_file_system(filename.c_str()));
		Profile prof;
		prof.read("map/elemental", nullptr, *l_fs);
		Section& s = prof.get_safe_section("global");

		std::string mapname = s.get_safe_string("name");
		nr_players_ = s.get_safe_int("nr_players");

		settings_->set_map(mapname, filename, nr_players_, true);

		// Check for sendability
		if (g_fs->is_directory(filename)) {
			// Send a warning
			UI::WLMessageBox warning(
			   this, _("Saved game is directory"),
			   _("WARNING:\n"
			     "The saved game you selected is a directory."
			     " This happens if you set the option ‘nozip’ to "
			     "true or manually unzipped the saved game.\n"
			     "Widelands is not able to transfer directory structures to the clients,"
			     " please select another saved game or zip the directories’ content."),
			   UI::WLMessageBox::MBoxType::kOk);
			warning.run<UI::Panel::Returncodes>();
		}
	}
}

/**
 * start-button has been pressed
 */
void FullscreenMenuLaunchMPG::clicked_ok() {
	if (!g_fs->file_exists(settings_->settings().mapfilename))
		throw WLWarning(_("File not found"),
		                _("Widelands tried to start a game with a file that could not be "
		                  "found at the given path.\n"
		                  "The file was: %s\n"
		                  "If this happens in a network game, the host might have selected "
		                  "a file that you do not own. Normally, such a file should be sent "
		                  "from the host to you, but perhaps the transfer was not yet "
		                  "finished!?!"),
		                settings_->settings().mapfilename.c_str());
	if (settings_->can_launch()) {
		settings_->set_win_condition_script(win_condition_dropdown_.get_selected());  // NOCOM new
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kNormalGame);
	}
}

/**
 * update the user interface and take care about the visibility of
 * buttons and text.
 */
void FullscreenMenuLaunchMPG::refresh() {
	const GameSettings& settings = settings_->settings();

	if (settings.mapfilename != filename_proof_) {
		if (!g_fs->file_exists(settings.mapfilename)) {
			client_info_.set_color(UI_FONT_CLR_WARNING);
			client_info_.set_text(
			   _("The selected file can not be found. If it is not automatically "
			     "transferred to you, please write to the host about this problem."));
		} else {
			// Reset font color
			client_info_.set_color(UI_FONT_CLR_FG);

			// Update local nr of players - needed for the client UI
			nr_players_ = settings.players.size();

			// Care about the newly selected file. This has to be done here and not
			// after selection of a new map / saved game, as the clients user
			// interface can only notice the change after the host broadcasted it.
			if (settings.savegame) {
				load_previous_playerdata();
			} else {
				load_map_info();
				if (settings.scenario)
					set_scenario_values();
			}
			// Try to translate the map name.
			// This will work on every official map as expected
			// and 'fail silently' (not find a translation) for already translated campaign map names.
			// It will also translate 'false-positively' on any user-made map which shares a name with
			// the official maps, but this should not be a problem to worry about.
			i18n::Textdomain td("maps");
			mapname_.set_text(_(settings.mapname));
		}
	} else {
		// Write client infos
		std::string client_info =
		   (settings.playernum >= 0) && (settings.playernum < kMaxPlayers) ?
		      (boost::format(_("You are Player %i.")) % (settings.playernum + 1)).str() :
		      _("You are a spectator.");
		client_info_.set_text(client_info);
	}

	ok_.set_enabled(settings_->can_launch());

	change_map_or_save_.set_enabled(settings_->can_change_map());
	change_map_or_save_.set_visible(settings_->can_change_map());

	if (!settings_->can_change_map()) {
		// update_win_conditions(); // NOCOM new
		// win_condition_dropdown_.set_label("Foo NOCOM");
		if (settings_->settings().scenario) {
			win_condition_dropdown_.set_label(_("Scenario"));
			win_condition_dropdown_.set_tooltip(_("Win condition is set through the scenario"));
		} else if (settings_->settings().savegame) {  // NOCOM duplication
			/** Translators: This is a game type */
			win_condition_dropdown_.set_label(_("Saved Game"));
			win_condition_dropdown_.set_tooltip(
			   _("The game is a saved game – the win condition was set before."));
		} else {
			try {
				std::unique_ptr<LuaTable> t = win_condition_if_valid(
				   settings_->get_win_condition_script(), std::set<std::string>());
				if (t) {
					i18n::Textdomain td("win_conditions");
					win_condition_dropdown_.set_label(_(t->get_string("name")));
					win_condition_dropdown_.set_tooltip(_(t->get_string("description")));
				}
			} catch (LuaTableKeyError& e) {
				log("LaunchMPG: Error loading win condition: %s %s\n",
				    settings_->get_win_condition_script().c_str(), e.what());
			}
		}
		win_condition_dropdown_.set_enabled(false);
	}
	// Update the multi player setup group
	mpsg_->refresh();
}

/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void FullscreenMenuLaunchMPG::set_scenario_values() {
	const GameSettings& settings = settings_->settings();
	if (settings.mapfilename.empty())
		throw wexception("settings()->scenario was set to true, but no map is available");
	Widelands::Map map;  //  MapLoader needs a place to put its preload data
	std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(settings.mapfilename));
	map.set_filename(settings.mapfilename);
	ml->preload_map(true);
	Widelands::PlayerNumber const nrplayers = map.get_nrplayers();
	for (uint8_t i = 0; i < nrplayers; ++i) {
		settings_->set_player_tribe(i, map.get_scenario_player_tribe(i + 1));
		settings_->set_player_closeable(i, map.get_scenario_player_closeable(i + 1));
		std::string ai(map.get_scenario_player_ai(i + 1));
		if (!ai.empty()) {
			settings_->set_player_state(i, PlayerSettings::stateComputer);
			settings_->set_player_ai(i, ai);
		} else if (settings.players.at(i).state != PlayerSettings::stateHuman &&
		           settings.players.at(i).state != PlayerSettings::stateOpen) {
			settings_->set_player_state(i, PlayerSettings::stateOpen);
		}
	}
}

/**
 * load all playerdata from savegame and update UI accordingly
 */
void FullscreenMenuLaunchMPG::load_previous_playerdata() {
	std::unique_ptr<FileSystem> l_fs(
	   g_fs->make_sub_file_system(settings_->settings().mapfilename.c_str()));
	Profile prof;
	prof.read("map/player_names", nullptr, *l_fs);
	std::string infotext = _("Saved players are:");
	std::string player_save_name[kMaxPlayers];
	std::string player_save_tribe[kMaxPlayers];
	std::string player_save_ai[kMaxPlayers];

	uint8_t i = 1;
	for (; i <= nr_players_; ++i) {
		infotext += "\n* ";
		Section& s =
		   prof.get_safe_section((boost::format("player_%u") % static_cast<unsigned int>(i)).str());
		player_save_name[i - 1] = s.get_string("name");
		player_save_tribe[i - 1] = s.get_string("tribe");
		player_save_ai[i - 1] = s.get_string("ai");

		infotext += (boost::format(_("Player %u")) % static_cast<unsigned int>(i)).str();
		if (player_save_tribe[i - 1].empty()) {
			std::string closed_string = (boost::format("<%s>") % _("closed")).str();
			infotext += ":\n    ";
			infotext += closed_string;
			// Close the player
			settings_->set_player_state(i - 1, PlayerSettings::stateClosed);
			continue;  // if tribe is empty, the player does not exist
		}

		// Set team to "none" - to get the real team, we would need to load the savegame completely
		// Do we want that? No! So we just reset teams to not confuse the clients.
		settings_->set_player_team(i - 1, 0);

		if (player_save_ai[i - 1].empty()) {
			// Assure that player is open
			if (settings_->settings().players.at(i - 1).state != PlayerSettings::stateHuman)
				settings_->set_player_state(i - 1, PlayerSettings::stateOpen);
		} else {
			settings_->set_player_state(i - 1, PlayerSettings::stateComputer);
			settings_->set_player_ai(i - 1, player_save_ai[i - 1]);
		}

		// Set player's tribe
		settings_->set_player_tribe(i - 1, player_save_tribe[i - 1]);

		// get translated tribename
		for (const TribeBasicInfo& tribeinfo : settings_->settings().tribes) {
			if (tribeinfo.name == player_save_tribe[i - 1]) {
				i18n::Textdomain td("tribes");  // for translated initialisation
				player_save_tribe[i - 1] = _(tribeinfo.descname);
				break;
			}
		}

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
	map_info_.set_text(infotext);
	filename_proof_ = settings_->settings().mapfilename;
}

/**
 * load map information and update the UI
 */
void FullscreenMenuLaunchMPG::load_map_info() {
	Widelands::Map map;  //  MapLoader needs a place to put its preload data

	std::unique_ptr<Widelands::MapLoader> ml =
	   map.get_correct_loader(settings_->settings().mapfilename);
	if (!ml) {
		throw WLWarning("There was an error!", "The map file seems to be invalid!");
	}

	map.set_filename(settings_->settings().mapfilename);
	{
		i18n::Textdomain td("maps");
		ml->preload_map(true);
	}

	std::string infotext;
	infotext += std::string(_("Map details:")) + "\n";
	infotext += std::string("• ") +
	            (boost::format(_("Size: %1$u x %2$u")) % map.get_width() % map.get_height()).str() +
	            "\n";
	infotext +=
	   std::string("• ") +
	   (boost::format(ngettext("%u Player", "%u Players", nr_players_)) % nr_players_).str() + "\n";
	if (settings_->settings().scenario)
		infotext += std::string("• ") + (boost::format(_("Scenario mode selected"))).str() + "\n";
	infotext += "\n";
	infotext += map.get_description();
	infotext += "\n";
	infotext += map.get_hint();

	map_info_.set_text(infotext);
	filename_proof_ = settings_->settings().mapfilename;

	suggested_teams_box_->hide();
	suggested_teams_box_->show(map.get_suggested_teams());
	suggested_teams_box_->set_pos(
	   Vector2i(suggested_teams_box_->get_x(),
	            back_.get_y() - padding_ - suggested_teams_box_->get_h() - padding_));
}

/// Show help
void FullscreenMenuLaunchMPG::help_clicked() {
	if (help_) {
		help_->set_visible(true);
	} else {
		help_.reset(
		   new UI::FullscreenHelpWindow(this, lua_, "txts/help/multiplayer_help.lua",
		                                /** TRANSLATORS: This is a heading for a help window */
		                                _("Multiplayer Game Setup")));
	}
}
