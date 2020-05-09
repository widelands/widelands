#include "launch_spg2.h"

#include <memory>

#include "base/i18n.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "graphic/font_handler.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/map_objects/map_object.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "mapselect.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_fsmenu/loadgame.h"

FullscreenMenuLaunchSPG2::FullscreenMenuLaunchSPG2(GameSettingsProvider* const settings,
                                                   GameController* const ctrl)
   : FullscreenMenuLaunchGame(settings, ctrl),

     player_setup(&individual_content_box, settings, standard_element_height_) {

	individual_content_box.add(&player_setup, UI::Box::Resizing::kExpandBoth);

	title_.set_text("Launch game");

	ok_.set_enabled(settings_->can_launch());

	subscriber_ = Notifications::subscribe<NoteGameSettings>(
	   [this](const NoteGameSettings& note) { update(false); });
}

/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void FullscreenMenuLaunchSPG2::start() {
	//	if (!select_map()) {
	//		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	//	}
}

/**
 * Select a map and send all information to the user interface.
 * Returns whether a map has been selected.
 */
bool FullscreenMenuLaunchSPG2::clicked_select_map() {
	if (!settings_->can_change_map())
		return false;

	FullscreenMenuMapSelect msm(settings_, nullptr);
	FullscreenMenuBase::MenuTarget code = msm.run<FullscreenMenuBase::MenuTarget>();

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		// Set scenario = false, else the menu might crash when back is pressed.
		settings_->set_scenario(false);
		return false;  // back was pressed
	}

	// why can't I ask settings_ if it is a scenario?? stupid to provide an extra bool field...
	is_scenario_ = code == FullscreenMenuBase::MenuTarget::kScenarioGame;
	settings_->set_scenario(is_scenario_);

	const MapData& mapdata = *msm.get_map();
	nr_players_ = mapdata.nrplayers;

	ensure_valid_host_position(nr_players_);
	settings_->set_map(mapdata.name, mapdata.filename, nr_players_);
	update_win_conditions();
	update_peaceful_mode();

	// is this ok? actually I don't understand why this is not automatically sent in
	// settings_#set_map()...
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));
	return true;
}

/**
 * Called when a position-button was clicked.
 */
void FullscreenMenuLaunchSPG2::switch_to_position(uint8_t const pos) {
	settings_->set_player_number(pos);
	update(false);
}

/**
 * Check to avoid segfaults, if the player changes a map with less player
 * positions while being on a later invalid position.
 */
void FullscreenMenuLaunchSPG2::ensure_valid_host_position(uint8_t const newplayernumber) {
	GameSettings settings = settings_->settings();

	// Check whether the host would still keep a valid position and return if
	// yes.
	if (settings.playernum == UserSettings::none() || settings.playernum < newplayernumber)
		return;

	// Check if a still valid place is open.
	for (uint8_t i = 0; i < newplayernumber; ++i) {
		PlayerSettings position = settings.players.at(i);
		if (position.state == PlayerSettings::State::kOpen) {
			switch_to_position(i);
			return;
		}
	}

	// Kick player 1 and take the position
	settings_->set_player_state(0, PlayerSettings::State::kClosed);
	settings_->set_player_state(0, PlayerSettings::State::kOpen);
	switch_to_position(0);
}

/**
 * update the user interface and take care of the visibility of
 * buttons and text.
 */
void FullscreenMenuLaunchSPG2::update(bool) {
	log("update\n");
	const GameSettings& settings = settings_->settings();

	map_details.update(settings_);
	nr_players_ = settings.players.size();

	ok_.set_enabled(settings_->can_launch());

	peaceful_.set_state(settings_->is_peaceful_mode());

	set_player_names_and_tribes();
	//}

	//	// "Choose Position" Buttons in front of PlayerDescriptionGroups
	//	for (uint8_t i = 0; i < nr_players_; ++i) {
	//		pos_[i]->set_visible(true);
	//		const PlayerSettings& player = settings.players[i];
	//		pos_[i]->set_enabled(!is_scenario_ && (player.state == PlayerSettings::State::kOpen ||
	//		                                       player.state == PlayerSettings::State::kComputer));
	//	}
	//	for (uint32_t i = nr_players_; i < kMaxPlayers; ++i)
	//		pos_[i]->set_visible(false);

	//	// update the player description groups
	//	for (uint32_t i = 0; i < kMaxPlayers; ++i) {
	//		players_[i]->update();
	//	}
}

/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void FullscreenMenuLaunchSPG2::set_player_names_and_tribes() {
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
		const std::string playertribe = map.get_scenario_player_tribe(i + 1);
		if (playertribe.empty()) {
			// Set tribe selection to random
			settings_->set_player_tribe(i, "", true);
		} else {
			// Set tribe selection from map
			settings_->set_player_tribe(i, playertribe);
		}
	}
}

/**
 * back-button has been pressed
 */
void FullscreenMenuLaunchSPG2::clicked_back() {
	//  The following behaviour might look strange at first view, but for the
	//  user it seems as if the launchgame-menu is a child of mapselect and
	//  not the other way around - just end_modal(0); will be seen as bug
	//  from user point of view, so we reopen the mapselect-menu.
	//	if (!select_map()) {
	// No map has been selected: Go back to main menu
	return end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	//	}
	// update(true);
}

void FullscreenMenuLaunchSPG2::win_condition_selected() {
	if (win_condition_dropdown_.has_selection()) {
		last_win_condition_ = win_condition_dropdown_.get_selected();

		std::unique_ptr<LuaTable> t = lua_->run_script(last_win_condition_);
		t->do_not_warn_about_unaccessed_keys();
		peaceful_mode_forbidden_ = !t->get_bool("peaceful_mode_allowed");
		update_peaceful_mode();
	}
}

void FullscreenMenuLaunchSPG2::clicked_ok() {
	const std::string filename = settings_->settings().mapfilename;
	if (!g_fs->file_exists(filename))
		throw WLWarning(_("File not found"),
		                _("Widelands tried to start a game with a file that could not be "
		                  "found at the given path.\n"
		                  "The file was: %s\n"
		                  "If this happens in a network game, the host might have selected "
		                  "a file that you do not own. Normally, such a file should be sent "
		                  "from the host to you, but perhaps the transfer was not yet "
		                  "finished!?!"),
		                filename.c_str());
	if (settings_->can_launch()) {
		settings_->set_scenario(true);
		if (is_scenario_) {
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kScenarioGame);
		} else {
			if (win_condition_dropdown_.has_selection()) {
				settings_->set_win_condition_script(win_condition_dropdown_.get_selected());
			}
			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kNormalGame);
		}
	}
}

void FullscreenMenuLaunchSPG2::layout() {
	standard_element_width_ = get_w() / 4;
	standard_element_height_ = get_h() * 9 / 200;
	player_setup.force_new_dimensions(scale_factor(), standard_element_height_);

	FullscreenMenuLaunchGame::layout();
}

FullscreenMenuLaunchSPG2::~FullscreenMenuLaunchSPG2() {
}
