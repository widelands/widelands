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
     main_box_(this, 0, 0, UI::Box::Vertical, get_w(), get_h()),
     content_box_(&main_box_, 0, 0, UI::Box::Horizontal),
     button_box_(&main_box_, 0, 0, UI::Box::Horizontal),
     player_box_(&content_box_, 0, 0, UI::Box::Vertical),
     map_box_(&content_box_, 0, 0, UI::Box::Vertical),
     players_(&player_box_,
              0,
              0,
              0,
              0,
              "Players",
              UI::Align::kRight,
              g_gr->styles().font_style(UI::FontStyle::kFsGameSetupHeadings)),
     map_(&map_box_,
          0,
          0,
          0,
          0,
          "Map",
          UI::Align::kLeft,
          g_gr->styles().font_style(UI::FontStyle::kFsGameSetupHeadings)),
     peaceful_own_(&map_box_, Vector2i::zero(), _("my Peaceful mode")),
     ok_own_(&button_box_,
             "ok",
             0,
             0,
             butw_,
             buth_,
             UI::ButtonStyle::kFsMenuPrimary,
             _("my Start game")),
     back_own_(
        &button_box_, "back", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuSecondary, _("my Back")),
     title_own_(&main_box_,
                0,
                0,
                0,
                0,
                "my Launch Game blabla",
                UI::Align::kCenter,
                g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)) {
	// main_box_.set_layout_toplevel(true);

	main_box_.add(&title_own_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	//	title_.set_text(_("Launch Game"));

	content_box_.add(&player_box_);
	player_box_.add(&players_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	player_box_.set_scrolling(true);

	// content_box_.add_inf_space();

	content_box_.add(&map_box_);
	map_box_.add(&map_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	map_box_.add(&peaceful_own_, UI::Box::Resizing::kAlign, UI::Align::kLeft);

	// main_box_.add_inf_space();
	main_box_.add(&content_box_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	//	main_box_.add_inf_space();

	main_box_.add(&button_box_, UI::Box::Resizing::kFullSize);
	button_box_.add_inf_space();
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &ok_button() : &back_button(),
	                UI::Box::Resizing::kExpandBoth);
	button_box_.add_space(10);
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &back_button() : &ok_button(),
	                UI::Box::Resizing::kExpandBoth);

	players_.set_font_scale(scale_factor());
	map_.set_font_scale(scale_factor());

	// temporary...
	title_.set_visible(false);
	ok_.set_visible(false);
	back_.set_visible(false);
	peaceful_.set_visible(false);
	win_condition_dropdown_.set_visible(false);

	layout();
	layout();
}

void FullscreenMenuLaunchSPG2::layout() {
	log("w=%d, h=%d\n", get_w(), get_h());
	//	main_box_.set_desired_size(get_w(), get_h());
	main_box_.set_size(get_w(), get_h());
	log("main box: w=%d, h=%d, x=%d\n", main_box_.get_w(), main_box_.get_h(), main_box_.get_x());
	//	title_own_.set_text(_("my Start game blabla"));
	log("title: w=%d, h=%d, x=%d\n", title_own_.get_w(), title_own_.get_h(), title_own_.get_x());
	content_box_.set_desired_size(
	   main_box_.get_w(), main_box_.get_h() - title_own_.get_h() - 2 * ok_own_.get_h());
	log("content box: w=%d, h=%d, x=%d\n", content_box_.get_w(), content_box_.get_h(),
	    content_box_.get_x());
	player_box_.set_desired_size(content_box_.get_w() / 2, content_box_.get_h());
	log("player box: w=%d, h=%d, x=%d\n", player_box_.get_w(), player_box_.get_h(),
	    player_box_.get_x());
	map_box_.set_desired_size(content_box_.get_w() / 2, content_box_.get_h());
	log("map box: w=%d, h=%d, x=%d\n", map_box_.get_w(), map_box_.get_h(), map_box_.get_x());
	// map_.set_desired_size(map_box_.get_w(), 0);

	button_box_.set_desired_size(
	   main_box_.get_w(), main_box_.get_h() - title_own_.get_h() - content_box_.get_h());
	log("button box: w=%d, h=%d\n", button_box_.get_w(), button_box_.get_h());
	log("peaceful w=%d, h=%d, x=%d\n", peaceful_own_.get_w(), peaceful_own_.get_h(),
	    peaceful_own_.get_x());
	log("map w=%d, h=%d, x=%d\n", map_.get_w(), map_.get_h(), map_.get_x());
	log("sehe ich nicht oder \n\n");
}

UI::Button& FullscreenMenuLaunchSPG2::ok_button() {
	log("return ok_own\n");
	return ok_own_;
}
UI::Button& FullscreenMenuLaunchSPG2::back_button() {
	return back_own_;
}
/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void FullscreenMenuLaunchSPG2::start() {
	//	if (!select_map()) {
	//	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
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
