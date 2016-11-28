/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include <boost/algorithm/string/predicate.hpp>

#include "base/i18n.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "helper.h"
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
// NOCOM Clean up includes
FullscreenMenuLaunchGame::FullscreenMenuLaunchGame(GameSettingsProvider* const settings,
                                                   GameController* const ctrl)
   : FullscreenMenuBase(),

     // Values for alignment and size
     butw_(get_w() / 4),
     buth_(get_h() * 9 / 200),

     win_condition_dropdown_(this, 0, 0, butw_, get_h() - get_h() * 4 / 10 - buth_, ""),
     ok_(this,
         "ok",
         0,
         0,
         butw_,
         buth_,
         g_gr->images().get("images/ui_basic/but2.png"),
         _("Start game")),
     back_(this,
           "back",
           0,
           0,
           butw_,
           buth_,
           g_gr->images().get("images/ui_basic/but0.png"),
           _("Back")),
     // Text labels
     title_(this, get_w() / 2, get_h() / 25, "", UI::Align::kHCenter),
     // Variables and objects used in the menu
     settings_(settings),
     ctrl_(ctrl),
     nr_players_(0) {
	win_condition_dropdown_.selected.connect(
	   boost::bind(&FullscreenMenuLaunchGame::win_condition_selected, this));
	back_.sigclicked.connect(
	   boost::bind(&FullscreenMenuLaunchGame::clicked_back, boost::ref(*this)));
	ok_.sigclicked.connect(boost::bind(&FullscreenMenuLaunchGame::clicked_ok, boost::ref(*this)));

	lua_ = new LuaInterface();

	title_.set_fontsize(fs_big());
}

FullscreenMenuLaunchGame::~FullscreenMenuLaunchGame() {
	delete lua_;
}

void FullscreenMenuLaunchGame::think() {
	if (ctrl_)
		ctrl_->think();

	refresh();
}

/**
 * Fill the dropdown with the available win conditions.
 */
void FullscreenMenuLaunchGame::update_win_conditions() {
	win_condition_dropdown_.clear();
	win_condition_dropdown_.set_enabled(settings_->can_change_map() &&
	                                    !settings_->settings().savegame &&
													!settings_->settings().scenario);
	if (settings_->settings().scenario) {
		win_condition_dropdown_.set_label(_("Scenario"));
		win_condition_dropdown_.set_tooltip(_("Win condition is set through the scenario"));
	} else if (settings_->settings().savegame) {
		/** Translators: This is a game type */
		win_condition_dropdown_.set_label(_("Saved Game"));
		win_condition_dropdown_.set_tooltip(
		   _("The game is a saved game – the win condition was set before."));
	} else {
		win_condition_dropdown_.set_label("");
		win_condition_dropdown_.set_tooltip("");
		Widelands::Map map;
		std::unique_ptr<Widelands::MapLoader> ml =
		   map.get_correct_loader(settings_->settings().mapfilename);
		if (ml != nullptr) {
			ml->preload_map(true);
			load_win_conditions(map);
		} else {
			const std::string error_message =
			   (boost::format(_("Unable to determine valid win conditions because the map '%s' could "
			                    "not be loaded.")) %
			    settings_->settings().mapfilename)
			      .str();
			win_condition_dropdown_.set_label(_("Error"));
			win_condition_dropdown_.set_tooltip(error_message);
			log("Launch Game: No map loader: %s\n", error_message.c_str());
		}
		if (settings_->can_change_map()) {
			win_condition_selected();
		}
	}
}

void FullscreenMenuLaunchGame::load_win_conditions(const Widelands::Map& map) {
	try {
		const std::set<std::string> tags = map.get_tags();
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
				log("LaunchSPG: Error loading win condition: %s %s\n", win_condition_script.c_str(),
				    e.what());
			}
		}
	} catch (const std::exception& e) {
		const std::string error_message =
		   (boost::format(_("Unable to determine valid win conditions because the map '%s' "
		                    "could not be loaded.")) %
		    settings_->settings().mapfilename)
		      .str();
		win_condition_dropdown_.set_label(_("Error"));
		win_condition_dropdown_.set_tooltip(error_message);
		log("LaunchSPG: Exception: %s %s\n", error_message.c_str(), e.what());
	}
}

std::unique_ptr<LuaTable>
FullscreenMenuLaunchGame::win_condition_if_valid(const std::string& win_condition_script,
                                                 std::set<std::string> tags) const {
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
		log(
		   "LaunchSPG: Error loading win condition: %s %s\n", win_condition_script.c_str(), e.what());
	}
	if (!is_usable) {
		t.reset(nullptr);
	}
	return t;
}

// Implemented by subclasses
void FullscreenMenuLaunchGame::clicked_ok() {
	NEVER_HERE();
}

// Implemented by subclasses
void FullscreenMenuLaunchGame::clicked_back() {
	NEVER_HERE();
}
