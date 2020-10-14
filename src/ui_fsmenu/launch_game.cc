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

#include "ui_fsmenu/launch_game.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "map_io/map_loader.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/mapselect.h"

FullscreenMenuLaunchGame::FullscreenMenuLaunchGame(GameSettingsProvider* const settings,
                                                   GameController* const ctrl,
                                                   const bool preconfigured)
   : FullscreenMenuBase(),

     // Values for alignment and size
     standard_element_width_(get_w() / 3),
     standard_element_height_(get_h() * 9 / 200),
     padding_(4),

     main_box_(this, 0, 0, UI::Box::Vertical),
     content_box_(&main_box_, 0, 0, UI::Box::Horizontal),
     individual_content_box(&content_box_, 0, 0, UI::Box::Vertical),
     map_box_(&content_box_, 0, 0, UI::Box::Vertical),
     map_details(
        &map_box_, preconfigured, standard_element_width_, standard_element_height_, padding_),

     configure_game(&map_box_,
                    0,
                    0,
                    0,
                    0,
                    _("Configure this game"),
                    UI::Align::kCenter,
                    g_style_manager->font_style(UI::FontStyle::kFsGameSetupHeadings)),
     win_condition_dropdown_(&map_box_,
                             "dropdown_wincondition",
                             0,
                             0,
                             standard_element_width_,
                             10,  // max number of items
                             standard_element_height_,
                             "",
                             UI::DropdownType::kTextual,
                             UI::PanelStyle::kFsMenu,
                             UI::ButtonStyle::kFsMenuMenu),
     peaceful_(&map_box_, Vector2i::zero(), _("Peaceful mode")),
     custom_starting_positions_(&map_box_, Vector2i::zero(), _("Custom starting positions")),
     ok_(&map_box_,
         "ok",
         0,
         0,
         standard_element_width_,
         standard_element_height_,
         UI::ButtonStyle::kFsMenuPrimary,
         _("Start game")),
     back_(&map_box_,
           "back",
           0,
           0,
           standard_element_width_,
           standard_element_height_,
           UI::ButtonStyle::kFsMenuSecondary,
           _("Back")),
     // Text labels
     title_(&main_box_,
            0,
            0,
            0,
            0,
            "",
            UI::Align::kCenter,
            g_style_manager->font_style(UI::FontStyle::kFsMenuTitle)),
     // Variables and objects used in the menu
     settings_(settings),
     ctrl_(ctrl),
     peaceful_mode_forbidden_(false) {
	win_condition_dropdown_.selected.connect([this]() { win_condition_selected(); });
	peaceful_.changed.connect([this]() { toggle_peaceful(); });
	custom_starting_positions_.changed.connect([this]() { toggle_custom_starting_positions(); });
	back_.sigclicked.connect([this]() { clicked_back(); });
	ok_.sigclicked.connect([this]() { clicked_ok(); });

	lua_ = new LuaInterface();
	add_all_widgets();
	add_behaviour_to_widgets();

	layout();
}

FullscreenMenuLaunchGame::~FullscreenMenuLaunchGame() {
	delete lua_;
}

void FullscreenMenuLaunchGame::add_all_widgets() {
	main_box_.add_space(10 * padding_);
	main_box_.add(&title_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box_.add_space(2 * padding_);

	main_box_.add(&content_box_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(10 * padding_);

	content_box_.add_space(10 * padding_);
	content_box_.add(&individual_content_box, UI::Box::Resizing::kExpandBoth);
	content_box_.add_space(10 * padding_);
	content_box_.add(&map_box_, UI::Box::Resizing::kFullSize);
	content_box_.add_space(10 * padding_);

	map_box_.add(&map_details);
	map_box_.add_space(5 * padding_);

	map_box_.add(&configure_game, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	map_box_.add_space(3 * padding_);
	map_box_.add(&win_condition_dropdown_);
	map_box_.add_space(3 * padding_);
	map_box_.add(&peaceful_);
	map_box_.add_space(3 * padding_);
	map_box_.add(&custom_starting_positions_);

	map_box_.add_inf_space();
	map_box_.add(&ok_, UI::Box::Resizing::kFullSize);
	map_box_.add_space(2 * padding_);
	map_box_.add(&back_, UI::Box::Resizing::kFullSize);
}

void FullscreenMenuLaunchGame::add_behaviour_to_widgets() {
	win_condition_dropdown_.selected.connect([this]() { win_condition_selected(); });
	peaceful_.changed.connect([this]() { toggle_peaceful(); });

	ok_.sigclicked.connect([this]() { clicked_ok(); });
	back_.sigclicked.connect([this]() { clicked_back(); });
	map_details.set_select_map_action([this]() { clicked_select_map(); });
}
void FullscreenMenuLaunchGame::layout() {
	main_box_.set_size(get_w(), get_h());
	standard_element_width_ = get_w() / 3;
	standard_element_height_ = get_h() * 9 / 200;

	ok_.set_desired_size(standard_element_width_, standard_element_height_);
	back_.set_desired_size(standard_element_width_, standard_element_height_);
	win_condition_dropdown_.set_desired_size(standard_element_width_, standard_element_height_);
	custom_starting_positions_.set_desired_size(standard_element_width_, standard_element_height_);

	title_.set_font_scale(scale_factor());
	map_details.force_new_dimensions(
	   scale_factor(), standard_element_width_, standard_element_height_);

	configure_game.set_font_scale(scale_factor());
}

void FullscreenMenuLaunchGame::update_peaceful_mode() {
	bool forbidden =
	   peaceful_mode_forbidden_ || settings_->settings().scenario || settings_->settings().savegame;
	peaceful_.set_enabled(!forbidden && settings_->can_change_map());
	if (forbidden) {
		peaceful_.set_state(false);
	}
	if (settings_->settings().scenario) {
		peaceful_.set_tooltip(_("The relations between players are set by the scenario"));
	} else if (settings_->settings().savegame) {
		peaceful_.set_tooltip(_("The relations between players are set by the saved game"));
	} else if (peaceful_mode_forbidden_) {
		peaceful_.set_tooltip(_("The selected win condition does not allow peaceful matches"));
	} else {
		peaceful_.set_tooltip(_("Forbid fighting between players"));
	}
}

void FullscreenMenuLaunchGame::update_custom_starting_positions() {
	const bool forbidden = settings_->settings().scenario || settings_->settings().savegame;
	custom_starting_positions_.set_enabled(!forbidden && settings_->can_change_map());
	if (forbidden) {
		custom_starting_positions_.set_state(false);
	}
	if (settings_->settings().scenario) {
		custom_starting_positions_.set_tooltip(_("The starting positions are set by the scenario"));
	} else if (settings_->settings().savegame) {
		custom_starting_positions_.set_tooltip(_("The starting positions are set by the saved game"));
	} else {
		custom_starting_positions_.set_tooltip(_(
		   "Allow the players to choose their own starting positions at the beginning of the game"));
	}
}

bool FullscreenMenuLaunchGame::init_win_condition_label() {
	if (settings_->settings().scenario) {
		win_condition_dropdown_.set_enabled(false);
		win_condition_dropdown_.set_label(_("Scenario"));
		win_condition_dropdown_.set_tooltip(_("Win condition is set through the scenario"));
		return true;
	} else if (settings_->settings().savegame) {
		win_condition_dropdown_.set_enabled(false);
		/** Translators: This is a game type */
		win_condition_dropdown_.set_label(_("Saved Game"));
		win_condition_dropdown_.set_tooltip(
		   _("The game is a saved game – the win condition was set before."));
		return true;
	} else {
		win_condition_dropdown_.set_enabled(settings_->can_change_map());
		win_condition_dropdown_.set_label("");
		win_condition_dropdown_.set_tooltip("");
		return false;
	}
}

/**
 * Fill the dropdown with the available win conditions.
 */
void FullscreenMenuLaunchGame::update_win_conditions() {
	if (!init_win_condition_label()) {
		std::set<std::string> tags;
		if (!settings_->settings().mapfilename.empty()) {
			Widelands::Map map;
			std::unique_ptr<Widelands::MapLoader> ml =
			   map.get_correct_loader(settings_->settings().mapfilename);
			if (ml != nullptr) {
				ml->preload_map(true);
				tags = map.get_tags();
			}
		}
		load_win_conditions(tags);
	}
}

void FullscreenMenuLaunchGame::load_win_conditions(const std::set<std::string>& tags) {
	win_condition_dropdown_.clear();
	try {
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
				log_err("Launch Game: Error loading win condition: %s %s\n",
				        win_condition_script.c_str(), e.what());
			}
		}
	} catch (const std::exception& e) {
		const std::string error_message =
		   (boost::format(_("Unable to determine valid win conditions because the map '%s' "
		                    "could not be loaded.")) %
		    settings_->settings().mapfilename)
		      .str();
		win_condition_dropdown_.set_errored(error_message);
		log_err("Launch Game: Exception: %s %s\n", error_message.c_str(), e.what());
	}
}

std::unique_ptr<LuaTable>
FullscreenMenuLaunchGame::win_condition_if_valid(const std::string& win_condition_script,
                                                 const std::set<std::string>& tags) const {
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
		log_err("Launch Game: Error loading win condition: %s %s\n", win_condition_script.c_str(),
		        e.what());
	}
	if (!is_usable) {
		t.reset(nullptr);
	}
	return t;
}

void FullscreenMenuLaunchGame::toggle_peaceful() {
	settings_->set_peaceful_mode(peaceful_.get_state());
}

void FullscreenMenuLaunchGame::toggle_custom_starting_positions() {
	settings_->set_custom_starting_positions(custom_starting_positions_.get_state());
}
