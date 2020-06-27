#include "ui_fsmenu/launch_spg.h"

#include <memory>

#include "base/warning.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "map_io/map_loader.h"
#include "ui_fsmenu/mapselect.h"

FullscreenMenuLaunchSPG::FullscreenMenuLaunchSPG(GameSettingsProvider* const settings,
                                                 GameController* const ctrl)
   : FullscreenMenuLaunchGame(settings, ctrl),

     player_setup(&individual_content_box, settings, standard_element_height_, padding_) {

	individual_content_box.add(&player_setup, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	title_.set_text(_("Launch game"));

	ok_.set_enabled(settings_->can_launch());

	subscriber_ =
	   Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings&) { update(); });
}

/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void FullscreenMenuLaunchSPG::start() {
	if (!clicked_select_map()) {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	}
}

/**
 * Select a map and send all information to the user interface.
 * Returns whether a map has been selected.
 */
bool FullscreenMenuLaunchSPG::clicked_select_map() {
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

	// ensure_valid_host_position(nr_players_);
	settings_->set_map(mapdata.name, mapdata.filename, mapdata.nrplayers);
	update_win_conditions();
	update_peaceful_mode();

	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));

	// force layout so all boxes and textareas are forced to update
	layout();
	return true;
}

void FullscreenMenuLaunchSPG::update() {
	Widelands::Map map;  //  MapLoader needs a place to put its preload data
	std::unique_ptr<Widelands::MapLoader> map_loader(
	   map.get_correct_loader(settings_->settings().mapfilename));
	map.set_filename(settings_->settings().mapfilename);
	map_loader->preload_map(true);

	map_details.update(settings_, map);
	suggested_teams_box_.show(map.get_suggested_teams());
	ok_.set_enabled(settings_->can_launch());

	peaceful_.set_state(settings_->is_peaceful_mode());

	// set_player_names_and_tribes(map);
}

/**
 * if map was selected to be loaded as scenario, set all values like
 * player names and player tribes and take care about visibility
 * and usability of all the parts of the UI.
 */
void FullscreenMenuLaunchSPG::set_player_names_and_tribes(Widelands::Map& map) {
	if (settings_->settings().mapfilename.empty()) {
		throw wexception("settings()->scenario was set to true, but no map is available");
	}

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
void FullscreenMenuLaunchSPG::clicked_back() {
	//  The following behaviour might look strange at first view, but for the
	//  user it seems as if the launchgame-menu is a child of mapselect and
	//  not the other way around - just end_modal(0); will be seen as bug
	//  from user point of view, so we reopen the mapselect-menu.
	// if (!select_map()) {
	// No map has been selected: Go back to main menu
	return end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	// }
	// update(true);
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

void FullscreenMenuLaunchSPG::layout() {
	standard_element_width_ = get_w() / 4;
	standard_element_height_ = get_h() * 9 / 200;
	player_setup.force_new_dimensions(scale_factor(), standard_element_height_);

	FullscreenMenuLaunchGame::layout();
}

FullscreenMenuLaunchSPG::~FullscreenMenuLaunchSPG() {
}
