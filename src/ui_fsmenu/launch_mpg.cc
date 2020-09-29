/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "base/i18n.h"
#include "base/log.h"
#include "base/warning.h"
#include "graphic/playercolor.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/profile.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/helpwindow.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/mapselect.h"

/// Simple user interaction window for selecting either map, save or cancel
struct MapOrSaveSelectionWindow : public UI::Window {
	MapOrSaveSelectionWindow(UI::Panel* parent, GameController* gc, uint32_t w, uint32_t h)
	   : Window(parent,
	            UI::WindowStyle::kFsMenu,
	            "selection_window",
	            0,
	            0,
	            w,
	            h,
	            /** TRANSLATORS: Dialog box title for selecting between map or saved game for new
	               multiplayer game */
	            _("Please select")),
	     ctrl_(gc) {
		center_to_parent();

		uint32_t y = get_inner_h() / 10;
		uint32_t space = y;
		uint32_t butw = get_inner_w() - 2 * space;
		uint32_t buth = (get_inner_h() - 2 * space) / 5;
		UI::Button* btn =
		   new UI::Button(this, "map", space, y, butw, buth, UI::ButtonStyle::kFsMenuSecondary,
		                  _("Map"), _("Select a map"));
		btn->sigclicked.connect([this]() { pressedButton(MenuTarget::kNormalGame); });

		btn = new UI::Button(this, "saved_game", space, y + buth + space, butw, buth,
		                     UI::ButtonStyle::kFsMenuSecondary,
		                     /** Translators: This is a button to select a savegame */
		                     _("Saved Game"), _("Select a saved game"));
		btn->sigclicked.connect([this]() { pressedButton(MenuTarget::kScenarioGame); });

		btn =
		   new UI::Button(this, "cancel", space + butw / 4, y + 3 * buth + 2 * space, butw / 2, buth,
		                  UI::ButtonStyle::kFsMenuSecondary, _("Cancel"), _("Cancel selection"));
		btn->sigclicked.connect([this]() { pressedButton(MenuTarget::kBack); });
	}

	void think() override {
		if (ctrl_) {
			ctrl_->think();
		}
	}

	void pressedButton(MenuTarget i) {
		end_modal<MenuTarget>(i);
	}

private:
	GameController* ctrl_;
};

FullscreenMenuLaunchMPG::FullscreenMenuLaunchMPG(FullscreenMenuMain& fsmm,
GameSettingsProvider* const settings,
                                                 GameController* const ctrl,
                                                 ChatProvider& chat)
   : FullscreenMenuLaunchGame(fsmm, settings, ctrl),

     help_button_(this,
                  "help",
                  0,
                  0,
                  standard_element_height_,
                  standard_element_height_,
                  UI::ButtonStyle::kFsMenuSecondary,
                  g_image_cache->get("images/ui_basic/menu_help.png"),
                  _("Show the help window")),
     help_(nullptr),

     mpsg_(&individual_content_box, 0, 0, 0, 0, settings, standard_element_height_),
     chat_(&individual_content_box, 0, 0, 0, 0, chat, UI::PanelStyle::kFsMenu) {

	help_button_.sigclicked.connect([this]() { help_clicked(); });

	if (settings_->can_change_map()) {
		map_details.set_map_description_text(_("Please select a map or saved game."));
	} else {
		map_details.set_map_description_text(_("The host has not yet selected a map or saved game."));
	}
	ok_.set_enabled(settings_->can_launch());

	individual_content_box.add(&mpsg_, UI::Box::Resizing::kExpandBoth);
	individual_content_box.add(&chat_, UI::Box::Resizing::kExpandBoth);

	subscriber_ = Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings& s) {
		if (s.action == NoteGameSettings::Action::kMap) {
			map_changed();
		}
	});
	layout();

	// If we are the host, open the map or save selection menu at startup
	if (settings_->settings().usernum == 0 && settings_->settings().mapname.empty()) {
		clicked_select_map();
		// Try to associate the host with the first player
		if (!settings_->settings().players.empty()) {
			settings_->set_player_number(0);
		}
	}
}

FullscreenMenuLaunchMPG::~FullscreenMenuLaunchMPG() = default;

void FullscreenMenuLaunchMPG::layout() {
	FullscreenMenuLaunchGame::layout();
	// hardcode help button because it does not fit in any box, align it to the map button...
	help_button_.set_size(standard_element_height_, standard_element_height_);
	help_button_.set_pos(
	   Vector2i(get_inner_w() - standard_element_height_, 0));

	mpsg_.set_max_size(0, individual_content_box.get_h() / 2);

	mpsg_.force_new_dimensions(1.f, individual_content_box.get_w(),
	                           individual_content_box.get_h() / 2, standard_element_height_);

	// set focus to chat input
	chat_.focus_edit();
}

/**
 * back-button has been pressed
 */
void FullscreenMenuLaunchMPG::clicked_back() {
	end_modal<MenuTarget>(MenuTarget::kBack);
}

void FullscreenMenuLaunchMPG::win_condition_selected() {
	if (settings_->can_change_map() && win_condition_dropdown_.has_selection()) {
		settings_->set_win_condition_script(win_condition_dropdown_.get_selected());
		last_win_condition_ = win_condition_dropdown_.get_selected();

		std::unique_ptr<LuaTable> t = lua_->run_script(last_win_condition_);
		t->do_not_warn_about_unaccessed_keys();
		peaceful_mode_forbidden_ = !t->get_bool("peaceful_mode_allowed");
		update_peaceful_mode();
	}
}

/// Opens a popup window to select a map or saved game
bool FullscreenMenuLaunchMPG::clicked_select_map() {
	MapOrSaveSelectionWindow selection_window(this, ctrl_, get_w() / 3, get_h() / 4);
	auto result = selection_window.run<MenuTarget>();
	assert(result == MenuTarget::kNormalGame ||
	       result == MenuTarget::kScenarioGame ||
	       result == MenuTarget::kBack);
	if (result == MenuTarget::kNormalGame) {
		select_map();

	} else if (result == MenuTarget::kScenarioGame) {
		select_saved_game();
	}
	update_win_conditions();
	return true;
}

/**
 * Select a map and send all information to the user interface.
 */
void FullscreenMenuLaunchMPG::select_map() {
	if (!settings_->can_change_map()) {
		return;
	}

	FullscreenMenuMapSelect msm(fsmm_, settings_, ctrl_);
	MenuTarget code = msm.run<MenuTarget>();

	if (code == MenuTarget::kBack) {
		// Set scenario = false, else the menu might crash when back is pressed.
		settings_->set_scenario(false);
		return;
	}

	settings_->set_scenario(code == MenuTarget::kScenarioGame);

	const MapData& mapdata = *msm.get_map();
	settings_->set_map(mapdata.name, mapdata.filename, mapdata.nrplayers);
	map_changed();
}

/**
 * Select a multi player saved game and send all information to the user
 * interface.
 */
void FullscreenMenuLaunchMPG::select_saved_game() {
	if (!settings_->can_change_map()) {
		return;
	}

	Widelands::Game game;  // The place all data is saved to.
	FullscreenMenuLoadGame lsgm(fsmm_, game, settings_);
	MenuTarget code = lsgm.run<MenuTarget>();

	if (code == MenuTarget::kBack) {
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
		const int nr_players = s.get_safe_int("nr_players");

		settings_->set_map(mapname, filename, nr_players, true);
		map_changed();

		// Check for sendability
		if (g_fs->is_directory(filename)) {
			// Send a warning
			UI::WLMessageBox warning(
			   this, UI::WindowStyle::kFsMenu, _("Saved Game is Directory"),
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
	if (!g_fs->file_exists(settings_->settings().mapfilename)) {
		throw WLWarning(_("File not found"),
		                _("Widelands tried to start a game with a file that could not be "
		                  "found at the given path.\n"
		                  "The file was: %s\n"
		                  "If this happens in a network game, the host might have selected "
		                  "a file that you do not own. Normally, such a file should be sent "
		                  "from the host to you, but perhaps the transfer was not yet "
		                  "finished!?!"),
		                settings_->settings().mapfilename.c_str());
	}
	if (settings_->can_launch()) {
		if (win_condition_dropdown_.has_selection()) {
			settings_->set_win_condition_script(win_condition_dropdown_.get_selected());
		}
		end_modal<MenuTarget>(MenuTarget::kNormalGame);
	}
}

void FullscreenMenuLaunchMPG::think() {
	if (ctrl_) {
		ctrl_->think();
	}
	refresh();

	// unfocus chat window when other UI element has focus
	if (!chat_.has_focus()) {
		chat_.unfocus_edit();
	}
}

void FullscreenMenuLaunchMPG::map_changed() {
	const GameSettings& settings = settings_->settings();
	if (!g_fs->file_exists(settings.mapfilename)) {
		map_details.show_warning(
		   _("The selected file can not be found. If it is not automatically transferred to you, "
		     "please write to the host about this problem."));
	} else {
		// Care about the newly selected file. This has to be done here and not
		// after selection of a new map / saved game, as the clients user
		// interface can only notice the change after the host broadcasted it.
		if (settings.savegame) {
			load_previous_playerdata();
		} else {
			load_map_info();
			if (settings.scenario) {
				set_scenario_values();
			}
		}
	}
}

/**
 * update the user interface and take care about the visibility of
 * buttons and text.
 */
void FullscreenMenuLaunchMPG::refresh() {
	// TODO(GunChleoc): Investigate what we can handle with NoteGameSettings. Maybe we can get rid of
	// refresh() and thus think().

	ok_.set_enabled(settings_->can_launch());

	update_peaceful_mode();
	update_custom_starting_positions();
	custom_starting_positions_.set_state(settings_->get_custom_starting_positions());
	peaceful_.set_state(settings_->is_peaceful_mode());

	if (!settings_->can_change_map() && !init_win_condition_label()) {
		try {
			// We do not validate the scripts for the client - it's only a label.
			std::unique_ptr<LuaTable> t = lua_->run_script(settings_->get_win_condition_script());
			t->do_not_warn_about_unaccessed_keys();
			if (t) {
				i18n::Textdomain td("win_conditions");
				win_condition_dropdown_.set_label(_(t->get_string("name")));
				win_condition_dropdown_.set_tooltip(_(t->get_string("description")));
			}
		} catch (LuaScriptNotExistingError&) {
			win_condition_dropdown_.set_label(_("Error"));
			win_condition_dropdown_.set_tooltip(
			   (boost::format(_("Unable to load the win condition script file '%s'.")) %
			    settings_->get_win_condition_script())
			      .str());

		} catch (LuaTableKeyError& e) {
			log_err("LaunchMPG: Error loading win condition: %s %s\n",
			        settings_->get_win_condition_script().c_str(), e.what());
		}
		win_condition_dropdown_.set_enabled(false);
	}
}

/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void FullscreenMenuLaunchMPG::set_scenario_values() {
	const GameSettings& settings = settings_->settings();
	if (settings.mapfilename.empty()) {
		throw wexception("settings()->scenario was set to true, but no map is available");
	}
	Widelands::Map map;  //  MapLoader needs a place to put its preload data
	std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(settings.mapfilename));
	map.set_filename(settings.mapfilename);
	ml->preload_map(true);
	Widelands::PlayerNumber const nrplayers = map.get_nrplayers();
	if (settings.players.size() != nrplayers) {
		// Due to asynchronous notifications, the client can crash when an update is missing and the
		// number of players is wrong.
		return;
	}
	for (uint8_t i = 0; i < nrplayers; ++i) {
		settings_->set_player_tribe(i, map.get_scenario_player_tribe(i + 1));
		settings_->set_player_closeable(i, map.get_scenario_player_closeable(i + 1));
		const std::string& ai = map.get_scenario_player_ai(i + 1);
		if (!ai.empty()) {
			settings_->set_player_state(i, PlayerSettings::State::kComputer);
			settings_->set_player_ai(i, ai);
		} else if (settings.players.at(i).state != PlayerSettings::State::kHuman &&
		           settings.players.at(i).state != PlayerSettings::State::kOpen) {
			settings_->set_player_state(i, PlayerSettings::State::kOpen);
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

	std::string player_save_name[kMaxPlayers];
	std::string player_save_tribe[kMaxPlayers];
	std::string player_save_ai[kMaxPlayers];

	for (uint8_t i = 1; i <= settings_->settings().players.size(); ++i) {
		Section* s =
		   prof.get_section((boost::format("player_%u") % static_cast<unsigned int>(i)).str());
		if (s == nullptr) {
			// Due to asynchronous notifications, the client can crash on savegame change when number
			// of players goes down. So, we abort if the section does not exist to prevent crashes.
			return;
		}
		player_save_name[i - 1] = s->get_string("name");
		player_save_tribe[i - 1] = s->get_string("tribe");
		player_save_ai[i - 1] = s->get_string("ai");

		if (player_save_tribe[i - 1].empty()) {
			// Close the player
			settings_->set_player_state(i - 1, PlayerSettings::State::kClosed);
			continue;  // if tribe is empty, the player does not exist
		}

		// Set team to "none" - to get the real team, we would need to load the savegame completely
		// Do we want that? No! So we just reset teams to not confuse the clients.
		settings_->set_player_team(i - 1, 0);

		if (player_save_ai[i - 1].empty()) {
			// Assure that player is open
			if (settings_->settings().players.at(i - 1).state != PlayerSettings::State::kHuman) {
				settings_->set_player_state(i - 1, PlayerSettings::State::kOpen);
			}
		} else {
			settings_->set_player_state(i - 1, PlayerSettings::State::kComputer);
			settings_->set_player_ai(i - 1, player_save_ai[i - 1]);
		}

		settings_->set_player_tribe(i - 1, player_save_tribe[i - 1]);
		settings_->set_player_name(i - 1, player_save_name[i - 1]);
	}

	map_details.update_from_savegame(settings_);
}

/**
 * load map information and update the UI
 */
void FullscreenMenuLaunchMPG::load_map_info() {
	Widelands::Map map;  // MapLoader needs a place to put its preload data

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

	map_details.update(settings_, map);
}

/// Show help
void FullscreenMenuLaunchMPG::help_clicked() {
	UI::FullscreenHelpWindow help(get_parent(), lua_, "txts/help/multiplayer_help.lua",
	                              /** TRANSLATORS: This is a heading for a help window */
	                              _("Multiplayer Game Setup"));
	help.run<UI::Panel::Returncodes>();
}
