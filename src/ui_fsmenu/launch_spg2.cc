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
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/mapselect.h"
#include "wui/playerdescrgroup.h"

FullscreenMenuLaunchSPG2::FullscreenMenuLaunchSPG2(GameSettingsProvider* const settings,
                                                   GameController* const ctrl)
   : FullscreenMenuLaunchGame(settings, ctrl),

     players_(&individual_content_box,
              0,
              0,
              0,
              0,
              "Players",
              UI::Align::kRight,
              g_gr->styles().font_style(UI::FontStyle::kFsGameSetupHeadings)),

     player_name_(&individual_content_box,
                  0,
                  0,
                  0,
                  0,
                  "beispiel name von einem spieler",
                  UI::Align::kLeft,
                  g_gr->styles().font_style(UI::FontStyle::kLabel)) {

	//	add_all_widgets();
	title_.set_text("my Launch game");

	//	disable_parent_widgets();

	Notifications::subscribe<NoteGameSettings>(
	   [this](const NoteGameSettings& note) { update(false); });
}

// void FullscreenMenuLaunchSPG2::add_all_widgets() {
//	main_box_.add_space(2 * padding_);
//	main_box_.add(&title_own_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

//	main_box_.add(&content_box_, UI::Box::Resizing::kExpandBoth);

//	main_box_.add_space(get_w() / 3);
//	content_box_.add(&player_box_, UI::Box::Resizing::kExpandBoth);
//	player_box_.add(&players_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
//	player_box_.add(&player_name_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
//	content_box_.add_inf_space();

//	content_box_.add(&map_box_, UI::Box::Resizing::kExpandBoth);
//	map_box_.add(&map_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
//	map_box_.add(&map_details, UI::Box::Resizing::kFullSize);
//	map_box_.add(&peaceful_own_, UI::Box::Resizing::kAlign, UI::Align::kLeft);
//	map_box_.add(&win_condition_type, UI::Box::Resizing::kAlign, UI::Align::kCenter);
//	map_box_.add(&win_condition_dropdown_own_, UI::Box::Resizing::kAlign, UI::Align::kLeft);

//	//	main_box_.add_inf_space();

//	main_box_.add(&button_box_, UI::Box::Resizing::kFullSize);
//	button_box_.add_inf_space();
//	button_box_.add(
//	   UI::g_fh->fontset()->is_rtl() ? &ok_own_ : &back_own_, UI::Box::Resizing::kExpandBoth);
//	button_box_.add_space(10);
//	button_box_.add(
//	   UI::g_fh->fontset()->is_rtl() ? &back_own_ : &ok_own_, UI::Box::Resizing::kExpandBoth);

//	players_.set_font_scale(scale_factor());
//	map_.set_font_scale(scale_factor());
//	map_name_.set_font_scale(scale_factor());
//}

// void FullscreenMenuLaunchSPG2::disable_parent_widgets() {
//	// temporary...
//	title_.set_visible(false);
//	ok_.set_visible(false);
//	back_.set_visible(false);
//	peaceful_.set_visible(false);
//	win_condition_dropdown_.set_visible(false);
//}

void FullscreenMenuLaunchSPG2::add_behaviour_to_widgets() {
	//	map_details.set_select_map_action([this]() { select_map(); });
	//	win_condition_dropdown_own_.selected.connect([this]() { win_condition_selected(); });
	//	ok_.sigclicked.connect([this]() { clicked_ok(); });
	//	back_.sigclicked.connect([this]() { clicked_back(); });

	// subscriber_ =
}
// void FullscreenMenuLaunchSPG2::layout() {
//	log("w=%d, h=%d\n", get_w(), get_h());
//	//	main_box_.set_desired_size(get_w(), get_h());
//	main_box_.set_size(get_w(), get_h());
//	log("main box: w=%d, h=%d, x=%d\n", main_box_.get_w(), main_box_.get_h(), main_box_.get_x());
//	//	title_own_.set_text(_("my Start game blabla"));
//	log("title: w=%d, h=%d, x=%d\n", title_own_.get_w(), title_own_.get_h(), title_own_.get_x());
//	//	content_box_.set_desired_size(
//	//	   main_box_.get_w(), main_box_.get_h() - title_own_.get_h() - 2 * ok_own_.get_h());
//	log("content box: w=%d, h=%d, x=%d\n", content_box_.get_w(), content_box_.get_h(),
//	    content_box_.get_x());
//	//	player_box_.set_desired_size(content_box_.get_w() / 2, content_box_.get_h());
//	log("player box: w=%d, h=%d, x=%d\n", player_box_.get_w(), player_box_.get_h(),
//	    player_box_.get_x());
//	//	map_box_.set_desired_size(content_box_.get_w() / 2, content_box_.get_h());
//	log("map box: w=%d, h=%d, x=%d\n", map_box_.get_w(), map_box_.get_h(), map_box_.get_x());
//	// map_.set_desired_size(map_box_.get_w(), 0);

//	//	button_box_.set_desired_size(
//	//	   main_box_.get_w(), main_box_.get_h() - title_own_.get_h() - content_box_.get_h());
//	log("button box: w=%d, h=%d\n", button_box_.get_w(), button_box_.get_h());
//	log("peaceful w=%d, h=%d, x=%d\n", peaceful_own_.get_w(), peaceful_own_.get_h(),
//	    peaceful_own_.get_x());
//	log("map w=%d, h=%d, x=%d\n", map_.get_w(), map_.get_h(), map_.get_x());
//	log("sehe ich nicht oder \n\n");
//	//	select_map_.set_desired_size(map_name_.get_h(), map_name_.get_h());
//}

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
	log("clicked the map button\n");
	if (!settings_->can_change_map())
		return false;

	FullscreenMenuMapSelect msm(settings_, nullptr);
	FullscreenMenuBase::MenuTarget code = msm.run<FullscreenMenuBase::MenuTarget>();

	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		// Set scenario = false, else the menu might crash when back is pressed.
		settings_->set_scenario(false);
		return false;  // back was pressed
	}

	//	is_scenario_ = code == FullscreenMenuBase::MenuTarget::kScenarioGame;
	//	settings_->set_scenario(is_scenario_);

	const MapData& mapdata = *msm.get_map();
	nr_players_ = mapdata.nrplayers;

	//	safe_place_for_host(nr_players_);
	settings_->set_map(mapdata.name, mapdata.filename, nr_players_);
	update_win_conditions();
	update_peaceful_mode();
	update(true);
	return true;
}

/**
 * update the user interface and take care of the visibility of
 * buttons and text.
 */
void FullscreenMenuLaunchSPG2::update(bool) {
	const GameSettings& settings = settings_->settings();

	//	map_details.update(settings_);
	//			filename_ = settings.mapfilename;
	nr_players_ = settings.players.size();

	ok_.set_enabled(settings_->can_launch());

	peaceful_.set_state(settings_->is_peaceful_mode());

	//		set_player_names_and_tribes();
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
	//	if (!g_fs->file_exists(filename_))
	//		throw WLWarning(_("File not found"),
	//		                _("Widelands tried to start a game with a file that could not be "
	//		                  "found at the given path.\n"
	//		                  "The file was: %s\n"
	//		                  "If this happens in a network game, the host might have selected "
	//		                  "a file that you do not own. Normally, such a file should be sent "
	//		                  "from the host to you, but perhaps the transfer was not yet "
	//		                  "finished!?!"),
	//		                filename_.c_str());
	//	if (settings_->can_launch()) {
	//		if (is_scenario_) {
	//			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kScenarioGame);
	//		} else {
	//			if (win_condition_dropdown_.has_selection()) {
	//				settings_->set_win_condition_script(win_condition_dropdown_.get_selected());
	//			}
	//			end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kNormalGame);
	//		}
	//	}
}

FullscreenMenuLaunchSPG2::~FullscreenMenuLaunchSPG2() {
}
