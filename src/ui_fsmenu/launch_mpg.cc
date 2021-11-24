/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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
#include "io/filesystem/layered_filesystem.h"
#include "io/profile.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player.h"
#include "logic/single_player_game_settings_provider.h"
#include "map_io/map_loader.h"
#include "network/internet_gaming.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/helpwindow.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/main.h"
#include "ui_fsmenu/mapselect.h"

namespace FsMenu {

LaunchMPG::LaunchMPG(MenuCapsule& fsmm,
                     GameSettingsProvider& settings,
                     GameController& ctrl,
                     ChatProvider& chat,
                     Widelands::Game& g,
                     bool game_done_on_cancel,
                     const std::function<void()>& c)
   : LaunchGame(fsmm, settings, &ctrl, false, true),
     callback_(c),
     game_done_on_cancel_(game_done_on_cancel),
     help_button_(this,
                  "help",
                  0,
                  0,
                  standard_height_,
                  standard_height_,
                  UI::ButtonStyle::kFsMenuSecondary,
                  g_image_cache->get("images/ui_basic/menu_help.png"),
                  _("Show the help window")),
     help_(nullptr),

     mpsg_(this, &left_column_box_, 0, 0, 0, 0, &settings, scale_factor * standard_height_),
     chat_(new GameChatPanel(&left_column_box_, 0, 0, 0, 0, chat, UI::PanelStyle::kFsMenu)),
     game_(g) {

	help_button_.sigclicked.connect([this]() { help_clicked(); });
	chat_->aborted.connect([this]() { die(); });

	if (settings_.can_change_map()) {
		map_details_.set_map_description_text(_("Please select a map or saved game."));
	} else {
		map_details_.set_map_description_text(
		   _("The host has not yet selected a map or saved game."));
	}
	ok_.set_enabled(settings_.can_launch());

	left_column_box_.add(&mpsg_, UI::Box::Resizing::kExpandBoth);
	left_column_box_.add_space(kPadding);
	left_column_box_.add(chat_.get(), UI::Box::Resizing::kExpandBoth);

	subscriber_ = Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings& s) {
		if (s.action == NoteGameSettings::Action::kMap) {
			map_changed();
		}
	});
	layout();
	initialization_complete();
}

LaunchMPG::~LaunchMPG() {
	if (game_done_on_cancel_) {
		InternetGaming::ref().set_game_done();
	}

	chat_.reset();  // do this early to avoid heap-use-after-free
}

void LaunchMPG::layout() {
	LaunchGame::layout();
	// hardcode help button because it does not fit in any box, place it in top right corner
	help_button_.set_size(standard_height_, standard_height_);
	help_button_.set_pos(Vector2i(get_inner_w() - help_button_.get_w(), 0));

	mpsg_.set_max_size(0, left_column_box_.get_h() / 2);

	mpsg_.force_new_dimensions(
	   left_column_box_.get_w(), left_column_box_.get_h() / 2, scale_factor * standard_height_);

	// set focus to chat input
	if (chat_) {
		chat_->focus_edit();
	}
}

void LaunchMPG::win_condition_selected() {
	if (settings_.can_change_map() && win_condition_dropdown_.has_selection()) {
		settings_.set_win_condition_script(win_condition_dropdown_.get_selected());
		last_win_condition_ = win_condition_dropdown_.get_selected();

		std::unique_ptr<LuaTable> t = lua_->run_script(last_win_condition_);
		t->do_not_warn_about_unaccessed_keys();
		peaceful_mode_forbidden_ = !t->get_bool("peaceful_mode_allowed");
		update_peaceful_mode();
		mpsg_.update_players();
	}
}

/**
 * Select a map and send all information to the user interface.
 */
void LaunchMPG::clicked_select_map() {
	if (settings_.can_change_map()) {
		new MapSelect(get_capsule(), this, &settings_, ctrl_, game_);
	}
}

void LaunchMPG::clicked_select_map_callback(const MapData* map, const bool scenario) {
	if (!map) {
		// Set scenario = false, else the menu might crash when back is pressed.
		settings_.set_scenario(false);
		return;
	}

	settings_.set_scenario(scenario);
	settings_.set_map(map->name, map->filename, map->theme, map->background, map->nrplayers);

	map_changed();
	update_win_conditions();
}

/**
 * Select a multi player saved game and send all information to the user
 * interface.
 */
void LaunchMPG::clicked_select_savegame() {
	if (!settings_.can_change_map()) {
		return;
	}

	Widelands::Game game;  // The place all data is saved to.
	new LoadGame(capsule_, game, settings_, false, false, [this](std::string filename) {
		// Saved game was selected - therefore not a scenario
		settings_.set_scenario(false);

		if (g_fs->file_exists(filename)) {
			// Read the needed data from file "elemental" of the used map.
			std::unique_ptr<FileSystem> l_fs(g_fs->make_sub_file_system(filename));
			Profile prof;
			prof.read("map/elemental", nullptr, *l_fs);
			Section& s = prof.get_safe_section("global");

			std::string mapname = s.get_safe_string("name");
			const int nr_players = s.get_safe_int("nr_players");

			settings_.set_map(mapname, filename, s.get_string("theme", ""),
			                  s.get_string("background", ""), nr_players, true);
			map_changed();

			// Check for sendability
			if (g_fs->is_directory(filename)) {
				// Send a warning
				UI::WLMessageBox warning(
				   &capsule_.menu(), UI::WindowStyle::kFsMenu, _("Saved Game is Directory"),
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
		update_win_conditions();
	});
}

/**
 * start-button has been pressed
 */
void LaunchMPG::clicked_ok() {
	if (!g_fs->file_exists(settings_.settings().mapfilename)) {
		throw WLWarning(_("File not found"),
		                _("Widelands tried to start a game with a file that could not be "
		                  "found at the given path.\n"
		                  "The file was: %s\n"
		                  "If this happens in a network game, the host might have selected "
		                  "a file that you do not own. Normally, such a file should be sent "
		                  "from the host to you, but perhaps the transfer was not yet "
		                  "finished!?!"),
		                settings_.settings().mapfilename.c_str());
	}
	if (settings_.can_launch()) {
		if (win_condition_dropdown_.has_selection()) {
			settings_.set_win_condition_script(win_condition_dropdown_.get_selected());
		}

		// Avoid heap-use-after-free because callback_() will delete the
		// chat provider. Do this in a slightly roundabout way to ensure
		// that the panel hierarchy will notice the chat panel's death.
		chat_.release()->do_delete();

		callback_();
		return_to_main_menu();
	}
}

void LaunchMPG::think() {
	if (ctrl_) {
		ctrl_->think();
	}
	refresh();

	// unfocus chat window when other UI element has focus
	if (chat_ && !chat_->has_focus()) {
		chat_->unfocus_edit();
	}
}

void LaunchMPG::map_changed() {
	const GameSettings& settings = settings_.settings();
	if (!g_fs->file_exists(settings.mapfilename)) {
		map_details_.show_warning(
		   _("The selected file can not be found. If it is not automatically transferred to you, "
		     "please write to the host about this problem."));
	} else {
		// Care about the newly selected file. This has to be done here and not
		// after selection of a new map / saved game, as the clients user
		// interface can only notice the change after the host broadcasted it.
		if (settings.savegame) {
			load_previous_playerdata();
		} else if (!settings.mapfilename.empty()) {
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
void LaunchMPG::refresh() {
	// TODO(GunChleoc): Investigate what we can handle with NoteGameSettings. Maybe we can get rid of
	// refresh() and thus think().

	ok_.set_enabled(settings_.can_launch());

	update_peaceful_mode();
	update_custom_starting_positions();
	custom_starting_positions_.set_state(settings_.get_custom_starting_positions());
	peaceful_.set_state(settings_.is_peaceful_mode());

	if (!settings_.can_change_map() && !init_win_condition_label()) {
		try {
			// We do not validate the scripts for the client - it's only a label.
			std::unique_ptr<LuaTable> t = lua_->run_script(settings_.get_win_condition_script());
			t->do_not_warn_about_unaccessed_keys();
			i18n::Textdomain td("win_conditions");
			win_condition_dropdown_.set_label(_(t->get_string("name")));
			win_condition_dropdown_.set_tooltip(_(t->get_string("description")));
		} catch (LuaScriptNotExistingError&) {
			win_condition_dropdown_.set_label(_("Error"));
			win_condition_dropdown_.set_tooltip(
			   bformat(_("Unable to load the win condition script file ‘%s’."),
			           settings_.get_win_condition_script()));

		} catch (LuaTableKeyError& e) {
			log_err("LaunchMPG: Error loading win condition: %s %s\n",
			        settings_.get_win_condition_script().c_str(), e.what());
		}
		win_condition_dropdown_.set_enabled(false);
	}
}

/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void LaunchMPG::set_scenario_values() {
	const GameSettings& settings = settings_.settings();
	if (settings.mapfilename.empty()) {
		throw wexception("settings()->scenario was set to true, but no map is available");
	}
	Widelands::Map map;  //  MapLoader needs a place to put its preload data
	std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(settings.mapfilename));
	map.set_filename(settings.mapfilename);
	ml->preload_map(true, nullptr);
	Widelands::PlayerNumber const nrplayers = map.get_nrplayers();
	if (settings.players.size() != nrplayers) {
		// Due to asynchronous notifications, the client can crash when an update is missing and the
		// number of players is wrong.
		return;
	}
	for (uint8_t i = 0; i < nrplayers; ++i) {
		settings_.set_player_closeable(i, map.get_scenario_player_closeable(i + 1));
		settings_.set_player_tribe(i, map.get_scenario_player_tribe(i + 1));
		const std::string& ai = map.get_scenario_player_ai(i + 1);
		if (!ai.empty()) {
			settings_.set_player_state(i, PlayerSettings::State::kComputer);
			settings_.set_player_ai(i, ai);
		} else if (settings.players.at(i).state != PlayerSettings::State::kHuman &&
		           settings.players.at(i).state != PlayerSettings::State::kOpen) {
			settings_.set_player_state(i, PlayerSettings::State::kOpen);
		}
	}
}

/**
 * load all playerdata from savegame and update UI accordingly
 */
void LaunchMPG::load_previous_playerdata() {
	std::unique_ptr<FileSystem> l_fs(g_fs->make_sub_file_system(settings_.settings().mapfilename));
	Profile prof;
	prof.read("map/player_names", nullptr, *l_fs);

	SinglePlayerGameSettingsProvider saved_settings;
	// Fill settings only with required data for the map details box
	saved_settings.set_map(
	   settings_.settings().mapname, "", "", "", settings_.settings().players.size(), true);

	for (uint8_t i = 1; i <= settings_.settings().players.size(); ++i) {
		Section* s = prof.get_section(bformat("player_%u", static_cast<unsigned int>(i)));
		if (s == nullptr) {
			// Due to asynchronous notifications, the client can crash on savegame change when number
			// of players goes down. So, we abort if the section does not exist to prevent crashes.
			return;
		}
		std::string player_save_name = s->get_string("name");
		std::string player_save_tribe = s->get_string("tribe");
		std::string player_save_ai = s->get_string("ai");
		Widelands::TeamNumber player_save_team = s->get_int("team");
		bool player_save_random = s->get_bool("random");

		if (player_save_tribe.empty()) {
			// Close the player
			settings_.set_player_state(i - 1, PlayerSettings::State::kClosed);
			saved_settings.set_player_state(i - 1, PlayerSettings::State::kClosed);
			continue;  // if tribe is empty, the player does not exist
		}

		settings_.set_player_team(i - 1, player_save_team);

		if (player_save_ai.empty()) {
			// Assure that player is open
			if (settings_.settings().players.at(i - 1).state != PlayerSettings::State::kHuman) {
				settings_.set_player_state(i - 1, PlayerSettings::State::kOpen);
				settings_.set_player_name(i - 1, player_save_name);
			}
		} else {
			settings_.set_player_state(i - 1, PlayerSettings::State::kComputer);
			settings_.set_player_ai(i - 1, player_save_ai);
			settings_.set_player_name(i - 1, player_save_name);
		}

		settings_.set_player_tribe(i - 1, player_save_tribe, player_save_random);
		saved_settings.set_player_tribe(i - 1, player_save_tribe, player_save_random);
		saved_settings.set_player_name(i - 1, player_save_name);
		saved_settings.set_player_state(i - 1, settings_.settings().players.at(i - 1).state);
	}

	map_details_.update_from_savegame(&saved_settings);
}

/**
 * load map information and update the UI
 */
void LaunchMPG::load_map_info() {
	Widelands::Map map;  // MapLoader needs a place to put its preload data

	std::unique_ptr<Widelands::MapLoader> ml =
	   map.get_correct_loader(settings_.settings().mapfilename);
	if (!ml) {
		throw WLWarning("There was an error!", "The map file seems to be invalid!");
	}

	map.set_filename(settings_.settings().mapfilename);
	{
		std::unique_ptr<i18n::GenericTextdomain> td(
		   AddOns::create_textdomain_for_map(settings_.settings().mapfilename));
		ml->preload_map(true, nullptr);
	}

	map_details_.update(&settings_, map);
}

/// Show help
void LaunchMPG::help_clicked() {
	HelpWindow help(&capsule_.menu(), lua_.get(), "txts/help/multiplayer_help.lua",
	                /** TRANSLATORS: This is a heading for a help window */
	                _("Multiplayer Game Setup"));
	help.run<UI::Panel::Returncodes>();
}
}  // namespace FsMenu
