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

#include "ui_fsmenu/launch_spg.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/random.h"
#include "base/time_string.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "build_info.h"
#include "config.h"
#include "editor/editorinteractive.h"
#include "editor/ui_menus/main_menu_random_map.h"
#include "graphic/default_resolution.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/mouse_cursor.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/game_settings.h"
#include "logic/map.h"
#include "logic/replay.h"
#include "logic/replay_game_controller.h"
#include "logic/single_player_game_controller.h"
#include "logic/single_player_game_settings_provider.h"
#include "map_io/map_loader.h"
#include "network/crypto.h"
#include "network/gameclient.h"
#include "network/gamehost.h"
#include "network/internet_gaming.h"
#include "sound/sound_handler.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/about.h"
#include "ui_fsmenu/campaign_select.h"
#include "ui_fsmenu/campaigns.h"
#include "ui_fsmenu/internet_lobby.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/main.h"
#include "ui_fsmenu/mapselect.h"
#include "ui_fsmenu/netsetup_lan.h"
#include "ui_fsmenu/options.h"
#include "ui_fsmenu/scenario_select.h"
#include "wlapplication.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"

namespace FsMenu {

LaunchSPG::LaunchSPG(MenuCapsule& fsmm,
                                                 GameSettingsProvider& settings,
                                                 Widelands::Game& g,
                                                 bool preconfigured)
   : LaunchGame(fsmm, settings, nullptr),
     player_setup(&left_column_box_, &settings, scale_factor * standard_height_, kPadding),
     preconfigured_(preconfigured),
     game_(&g),
     initializing_(true) {

	left_column_box_.add(&player_setup, UI::Box::Resizing::kExpandBoth);
	ok_.set_enabled(settings_->can_launch() || preconfigured_);

	if (preconfigured) {
		Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));
		update_win_conditions();
		update_peaceful_mode();
		update_custom_starting_positions();
		update();
		layout();
	} else {
		clicked_select_map();
	}
}

void LaunchSPG::clicked_select_map() {
	if (!preconfigured_ && settings_->can_change_map()) {
		new MapSelect(*this, settings_.get(), nullptr, *game_);
	}
}

void LaunchSPG::clicked_select_map_callback(const MapData* mapdata, const bool scenario) {
	assert(!preconfigured_ && settings_->can_change_map());

	if (!mapdata) {
		if (initializing_) {
			die();
		}
		return;
	}
	initializing_ = false;

	settings_->set_scenario(scenario);
	assert(!settings_->settings().tribes.empty());
	settings_->set_map(
	   mapdata->name, mapdata->filename, mapdata->theme, mapdata->background, mapdata->nrplayers);
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));

	update_win_conditions();
	update_peaceful_mode();
	update_custom_starting_positions();
	update();
	// force layout so all boxes and textareas are forced to update
	layout();
}

void LaunchSPG::update() {
	peaceful_.set_state(settings_->is_peaceful_mode());
	if (preconfigured_) {
		map_details.update(settings_.get(), *game_->mutable_map());
		ok_.set_enabled(true);
	} else {
		Widelands::Map map;  //  MapLoader needs a place to put its preload data
		std::unique_ptr<Widelands::MapLoader> map_loader(
		   map.get_correct_loader(settings_->settings().mapfilename));
		map.set_filename(settings_->settings().mapfilename);
		{
			i18n::Textdomain td("maps");
			map_loader->preload_map(true);
		}

		map_details.update(settings_.get(), map);
		ok_.set_enabled(settings_->can_launch());
		enforce_player_names_and_tribes(map);
	}
}

void LaunchSPG::enforce_player_names_and_tribes(const Widelands::Map& map) {
	if (settings_->settings().mapfilename.empty()) {
		throw wexception("settings()->scenario was set to true, but no map is available");
	}

	Widelands::PlayerNumber const nrplayers = map.get_nrplayers();
	for (uint8_t i = 0; i < nrplayers; ++i) {
		settings_->set_player_name(i, map.get_scenario_player_name(i + 1));
		const std::string& playertribe = map.get_scenario_player_tribe(i + 1);
		if (playertribe.empty()) {
			// Set tribe selection to random
			settings_->set_player_tribe(i, "", true);
		} else {
			// Set tribe selection from map
			settings_->set_player_tribe(i, playertribe);
		}
	}
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kPlayer));
}

void LaunchSPG::win_condition_selected() {
	if (win_condition_dropdown_.has_selection()) {
		last_win_condition_ = win_condition_dropdown_.get_selected();

		std::unique_ptr<LuaTable> t = lua_->run_script(last_win_condition_);
		t->do_not_warn_about_unaccessed_keys();
		peaceful_mode_forbidden_ = !t->get_bool("peaceful_mode_allowed");
		update_peaceful_mode();
	}
}

void LaunchSPG::clicked_ok() {
	const std::string filename = settings_->settings().mapfilename;
	if (!preconfigured_ && !g_fs->file_exists(filename)) {
		UI::WLMessageBox m(&capsule_.menu(), UI::WindowStyle::kFsMenu, _("File not found"),
		                (boost::format(
		                _("Widelands tried to start a game with a file that could not be "
		                  "found at the given path.\n"
		                  "The file was: %s\n"
		                  "If this happens in a network game, the host might have selected "
		                  "a file that you do not own. Normally, such a file should be sent "
		                  "from the host to you, but perhaps the transfer was not yet "
		                  "finished!?!"))
		                % filename).str(), UI::WLMessageBox::MBoxType::kOk);
		m.run<int>();
		return;
	}
	if (!settings_->can_launch() && !preconfigured_) {
		return;
	}

	capsule_.set_visible(false);
	Widelands::PlayerNumber playernumber = 1;
	upcast(SinglePlayerGameSettingsProvider, sp, settings_.get());
	assert(sp);
	game_->set_ai_training_mode(get_config_bool("ai_training", false));
	try {
		if (sp->settings().scenario) {  // scenario
			game_->run_splayer_scenario_direct(sp->get_map(), "");
		} else {  // normal singleplayer
			playernumber = sp->settings().playernum + 1;
			sp->set_win_condition_script(win_condition_dropdown_.get_selected());
			// Game controller needs the ibase pointer to init the chat
			game_->set_ibase(new InteractivePlayer(*game_, get_config_section(), playernumber, false));
			std::unique_ptr<GameController> ctrl(new SinglePlayerGameController(*game_, true, playernumber));

			std::vector<std::string> tipstexts{"general_game", "singleplayer"};
			if (sp->has_players_tribe()) {
				tipstexts.push_back(sp->get_players_tribe());
			}
			game_->create_loader_ui(tipstexts, false, sp->settings().map_theme, sp->settings().map_background);

			Notifications::publish(UI::NoteLoadingMessage(_("Preparing game…")));

			game_->set_game_controller(ctrl.get());
			game_->init_newgame(sp->settings());
			game_->run(Widelands::Game::StartGameType::kMap, "", false, "single_player");
		}
	} catch (const std::exception& e) {
		WLApplication::emergency_save(capsule_.menu(), *game_, e.what(), playernumber);
	}

	capsule_.menu().set_labels();  // Update the Continue button in case a new savegame was created
	die();
}

void LaunchSPG::layout() {
	LaunchGame::layout();
	player_setup.force_new_dimensions(scale_factor * standard_height_);
}

}  // namespace FsMenu
