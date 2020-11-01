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

#include "base/warning.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "ui_fsmenu/mapselect.h"

FullscreenMenuLaunchSPG::FullscreenMenuLaunchSPG(FullscreenMenuMain& fsmm,
                                                 GameSettingsProvider* const settings,
                                                 Widelands::EditorGameBase& egbase,
                                                 bool preconfigured,
                                                 GameController* const ctrl)
   : FullscreenMenuLaunchGame(fsmm, settings, ctrl),
     player_setup(
        &left_column_box_, settings, standard_height_ /*standard_element_height_*/, padding),
     preconfigured_(preconfigured),
     egbase_(egbase) {

	left_column_box_.add(&player_setup, UI::Box::Resizing::kExpandBoth);
	ok_.set_enabled(settings_->can_launch() || preconfigured_);

	if (preconfigured) {
		Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));
		update_win_conditions();
		update_peaceful_mode();
		update_custom_starting_positions();
		update();
		layout();
	}
}

/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void FullscreenMenuLaunchSPG::start() {
	if (!preconfigured_ && !clicked_select_map()) {
		end_modal<MenuTarget>(MenuTarget::kBack);
	}
}

/**
 * Select a map and send all information to the user interface.
 * Returns whether a map has been selected.
 */
bool FullscreenMenuLaunchSPG::clicked_select_map() {
	if (preconfigured_ || !settings_->can_change_map()) {
		return false;
	}

	set_visible(false);
	FullscreenMenuMapSelect msm(fsmm_, settings_, nullptr, egbase_);
	MenuTarget code = msm.run<MenuTarget>();
	set_visible(true);

	if (code == MenuTarget::kBack) {
		// Set scenario = false, else the menu might crash when back is pressed.
		settings_->set_scenario(false);
		return false;  // back was pressed
	}

	settings_->set_scenario(code == MenuTarget::kScenarioGame);

	const MapData& mapdata = *msm.get_map();

	assert(!settings_->settings().tribes.empty());

	settings_->set_map(
	   mapdata.name, mapdata.filename, mapdata.theme, mapdata.background, mapdata.nrplayers);
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));

	update_win_conditions();
	update_peaceful_mode();
	update_custom_starting_positions();
	update();

	// force layout so all boxes and textareas are forced to update
	layout();
	return true;
}

void FullscreenMenuLaunchSPG::update() {
	peaceful_.set_state(settings_->is_peaceful_mode());
	if (preconfigured_) {
		map_details.update(settings_, *egbase_.mutable_map());
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

		map_details.update(settings_, map);
		ok_.set_enabled(settings_->can_launch());
		enforce_player_names_and_tribes(map);
	}
}

void FullscreenMenuLaunchSPG::enforce_player_names_and_tribes(Widelands::Map& map) {
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

void FullscreenMenuLaunchSPG::clicked_back() {
	return end_modal<MenuTarget>(MenuTarget::kBack);
}

void FullscreenMenuLaunchSPG::win_condition_selected() {
	if (win_condition_dropdown_.has_selection()) {
		last_win_condition_ = win_condition_dropdown_.get_selected();

		std::unique_ptr<LuaTable> t = lua_->run_script(last_win_condition_);
		t->do_not_warn_about_unaccessed_keys();
		peaceful_mode_forbidden_ = !t->get_bool("peaceful_mode_allowed");
		update_peaceful_mode();
	}
}

void FullscreenMenuLaunchSPG::clicked_ok() {
	const std::string filename = settings_->settings().mapfilename;
	if (!preconfigured_ && !g_fs->file_exists(filename)) {
		throw WLWarning(_("File not found"),
		                _("Widelands tried to start a game with a file that could not be "
		                  "found at the given path.\n"
		                  "The file was: %s\n"
		                  "If this happens in a network game, the host might have selected "
		                  "a file that you do not own. Normally, such a file should be sent "
		                  "from the host to you, but perhaps the transfer was not yet "
		                  "finished!?!"),
		                filename.c_str());
	}
	if (settings_->can_launch() || preconfigured_) {
		if (settings_->settings().scenario) {
			end_modal<MenuTarget>(MenuTarget::kScenarioGame);
		} else {
			if (win_condition_dropdown_.has_selection()) {
				settings_->set_win_condition_script(win_condition_dropdown_.get_selected());
			}
			end_modal<MenuTarget>(MenuTarget::kNormalGame);
		}
	}
}

void FullscreenMenuLaunchSPG::layout() {
	FullscreenMenuLaunchGame::layout();
	player_setup.force_new_dimensions(standard_height_);
}

FullscreenMenuLaunchSPG::~FullscreenMenuLaunchSPG() {
}
