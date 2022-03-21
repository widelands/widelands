/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ui_fsmenu/launch_spg.h"

#include <memory>

#include "base/wexception.h"
#include "logic/game.h"
#include "logic/single_player_game_controller.h"
#include "logic/single_player_game_settings_provider.h"
#include "map_io/map_loader.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/main.h"
#include "ui_fsmenu/mapselect.h"
#include "wlapplication.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"

namespace FsMenu {

LaunchSPG::LaunchSPG(MenuCapsule& fsmm,
                     GameSettingsProvider& settings,
                     std::shared_ptr<Widelands::Game> g,
                     const MapData* mapdata,
                     const bool scenario)
   : LaunchGame(fsmm, settings, nullptr, !mapdata, false),
     player_setup_(&left_column_box_, *this, &settings, scale_factor * standard_height_),
     preconfigured_(!mapdata),
     game_(g) {

	left_column_box_.add(&player_setup_, UI::Box::Resizing::kExpandBoth);
	ok_.set_enabled(settings_.can_launch() || preconfigured_);

	settings_.set_scenario(scenario);
	if (!preconfigured_) {
		assert(!settings_.settings().tribes.empty());
		settings_.set_map(
		   mapdata->name, mapdata->filename, mapdata->theme, mapdata->background, mapdata->nrplayers);
	}
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));

	update_win_conditions();
	update_peaceful_mode();
	update_custom_starting_positions();
	update_warn_desyncing_addon();
	update();
	layout();
	initialization_complete();
}

void LaunchSPG::update() {
	peaceful_.set_state(settings_.is_peaceful_mode());
	if (preconfigured_) {
		map_details_.update(&settings_, *game_->mutable_map());
		ok_.set_enabled(true);
	} else {
		Widelands::Map map;  //  MapLoader needs a place to put its preload data
		std::unique_ptr<Widelands::MapLoader> map_loader(
		   map.get_correct_loader(settings_.settings().mapfilename));
		map.set_filename(settings_.settings().mapfilename);
		{
			std::unique_ptr<i18n::GenericTextdomain> td(
			   AddOns::create_textdomain_for_map(settings_.settings().mapfilename));
			map_loader->preload_map(true, nullptr);
		}

		map_details_.update(&settings_, map);
		ok_.set_enabled(settings_.can_launch());
		enforce_player_names_and_tribes(map);
	}
}

void LaunchSPG::enforce_player_names_and_tribes(const Widelands::Map& map) {
	if (settings_.settings().mapfilename.empty()) {
		throw wexception("settings()->scenario was set to true, but no map is available");
	}

	Widelands::PlayerNumber const nrplayers = map.get_nrplayers();
	for (uint8_t i = 0; i < nrplayers; ++i) {
		settings_.set_player_name(i, map.get_scenario_player_name(i + 1));
		const std::string& playertribe = map.get_scenario_player_tribe(i + 1);
		if (playertribe.empty()) {
			// Set tribe selection to random
			settings_.set_player_tribe(i, "", true);
		} else {
			// Set tribe selection from map
			settings_.set_player_tribe(i, playertribe);
		}
	}
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kPlayer));
}

void LaunchSPG::win_condition_selected() {
	if (win_condition_dropdown_.has_selection()) {
		settings_.set_win_condition_script(win_condition_dropdown_.get_selected());
		last_win_condition_ = win_condition_dropdown_.get_selected();

		std::unique_ptr<LuaTable> t = lua_->run_script(last_win_condition_);
		t->do_not_warn_about_unaccessed_keys();
		peaceful_mode_forbidden_ = !t->get_bool("peaceful_mode_allowed");
		update_peaceful_mode();
		player_setup_.update();
	}
}

void LaunchSPG::clicked_ok() {
	const std::string filename = settings_.settings().mapfilename;
	if (!preconfigured_ && !g_fs->file_exists(filename)) {
		UI::WLMessageBox m(
		   &capsule_.menu(), UI::WindowStyle::kFsMenu, _("File not found"),
		   format(_("Widelands tried to start a game with a file that could not be "
		            "found at the given path.\n"
		            "The file was: %s\n"
		            "If this happens in a network game, the host might have selected "
		            "a file that you do not own. Normally, such a file should be sent "
		            "from the host to you, but perhaps the transfer was not yet "
		            "finished!?!"),
		          filename),
		   UI::WLMessageBox::MBoxType::kOk);
		m.run<int>();
		return;
	}
	if (!settings_.can_launch() && !preconfigured_) {
		return;
	}

	capsule_.set_visible(false);
	Widelands::PlayerNumber playernumber = 1;
	upcast(SinglePlayerGameSettingsProvider, sp, &settings_);
	assert(sp);
	game_->set_ai_training_mode(get_config_bool("ai_training", false));
	try {
		if (sp->settings().scenario) {  // scenario
			game_->run_splayer_scenario_direct({sp->get_map()}, "");
		} else {  // normal singleplayer
			playernumber = sp->settings().playernum + 1;
			sp->set_win_condition_script(win_condition_dropdown_.get_selected());
			// Game controller needs the ibase pointer to init the chat
			game_->set_ibase(new InteractivePlayer(*game_, get_config_section(), playernumber, false));

			std::vector<std::string> tipstexts{"general_game", "singleplayer"};
			if (sp->has_players_tribe()) {
				tipstexts.push_back(sp->get_players_tribe());
			}
			game_->create_loader_ui(
			   tipstexts, true, sp->settings().map_theme, sp->settings().map_background);

			Notifications::publish(UI::NoteLoadingMessage(_("Preparing game…")));

			game_->set_game_controller(
			   std::make_shared<SinglePlayerGameController>(*game_, true, playernumber));
			game_->init_newgame(sp->settings());
			game_->run(Widelands::Game::StartGameType::kMap, "", false, "single_player");
		}
	} catch (const std::exception& e) {
		WLApplication::emergency_save(&capsule_.menu(), *game_, e.what(), playernumber);
	}

	return_to_main_menu();
}

void LaunchSPG::layout() {
	LaunchGame::layout();
	player_setup_.force_new_dimensions(
	   scale_factor * standard_height_, left_column_box_.get_inner_h());
}

}  // namespace FsMenu
