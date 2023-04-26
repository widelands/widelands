/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#include "ui_fsmenu/launch_game.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "map_io/map_loader.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_fsmenu/loadgame.h"
#include "ui_fsmenu/mapselect.h"

namespace FsMenu {

LaunchGame::LaunchGame(MenuCapsule& fsmm,
                       GameSettingsProvider& settings,
                       GameController* const ctrl,
                       const bool preconfigured,
                       const bool mpg)
   : TwoColumnsFullNavigationMenu(fsmm, _("Launch Game")),
     map_details_(&right_column_content_box_, kPadding),

     configure_game_(&right_column_content_box_,
                     UI::PanelStyle::kFsMenu,
                     UI::FontStyle::kFsGameSetupHeadings,
                     0,
                     0,
                     0,
                     0,
                     _("Configure this game"),
                     UI::Align::kCenter),
     warn_desyncing_addon_(
        &right_column_content_box_,
        0,
        0,
        10,
        10,
        UI::PanelStyle::kFsMenu,
        format("<rt><p>%s</p></rt>",
               g_style_manager->font_style(UI::FontStyle::kWarning)
                  .as_font_tag(
                     _("An enabled add-on is known to cause desyncs. No replay will be written."))),
        UI::Align::kLeft,
        UI::MultilineTextarea::ScrollMode::kNoScrolling),
     win_condition_dropdown_(&right_column_content_box_,
                             "dropdown_wincondition",
                             0,
                             0,
                             0,
                             10,  // max number of items
                             standard_height_,
                             "",
                             UI::DropdownType::kTextual,
                             UI::PanelStyle::kFsMenu,
                             UI::ButtonStyle::kFsMenuMenu),
     win_condition_duration_(&right_column_content_box_,
                             0,
                             0,
                             360,
                             240,
                             Widelands::kDefaultWinConditionDuration,
                             15,        // 15 minutes minimum gametime
                             512 * 60,  // 512 hours maximum gametime (arbitrary limit)
                             UI::PanelStyle::kFsMenu,
                             _("Playing time"),
                             UI::SpinBox::Units::kMinutes,
                             UI::SpinBox::Type::kBig,
                             5,
                             60),

     toggle_advanced_options_(&right_column_content_box_,
                              "toggle_advanced",
                              0,
                              0,
                              0,
                              ok_.get_h(),
                              UI::ButtonStyle::kFsMenuSecondary,
                              "" /* set later */,
                              _("Show or hide additional game configuration options")),
     advanced_options_box_(
        &right_column_content_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     game_flag_checkboxes_({
        {GameSettings::Flags::kPeaceful,
         {new UI::Checkbox(
             &advanced_options_box_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Peaceful mode")),
          &LaunchGame::update_peaceful_mode}},

        {GameSettings::Flags::kFogless,
         {new UI::Checkbox(
             &advanced_options_box_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("No fog of war")),
          &LaunchGame::update_fogless}},

        {GameSettings::Flags::kForbidDiplomacy,
         {new UI::Checkbox(&advanced_options_box_,
                           UI::PanelStyle::kFsMenu,
                           Vector2i::zero(),
                           _("Forbid diplomacy")),
          &LaunchGame::update_forbid_diplomacy}},

        {GameSettings::Flags::kCustomStartingPositions,
         {new UI::Checkbox(&advanced_options_box_,
                           UI::PanelStyle::kFsMenu,
                           Vector2i::zero(),
                           _("Custom starting positions")),
          &LaunchGame::update_custom_starting_positions}},

     }),
     write_replay_(
        &advanced_options_box_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Write Replay")),

     choose_map_(mpg && settings.can_change_map() && !preconfigured ?
                    new UI::Button(&right_column_content_box_,
                                   "choose_map",
                                   0,
                                   0,
                                   0,
                                   ok_.get_h(),
                                   UI::ButtonStyle::kFsMenuSecondary,
                                   _("Change map…")) :
                    nullptr),
     choose_savegame_(mpg && settings.can_change_map() && !preconfigured ?
                         new UI::Button(&right_column_content_box_,
                                        "choose_savegame",
                                        0,
                                        0,
                                        0,
                                        ok_.get_h(),
                                        UI::ButtonStyle::kFsMenuSecondary,
                                        _("Change saved game…")) :
                         nullptr),

     // Variables and objects used in the menu
     settings_(settings),
     ctrl_(ctrl) {
	warn_desyncing_addon_.set_visible(false);
	write_replay_.set_state(true);

	win_condition_dropdown_.selected.connect([this]() { win_condition_selected(); });
	win_condition_duration_.changed.connect([this]() { win_condition_duration_changed(); });
	for (auto& pair : game_flag_checkboxes_) {
		pair.second.first->changed.connect(
		   [this, pair]() { settings_.set_flag(pair.first, pair.second.first->get_state()); });
	}
	toggle_advanced_options_.sigclicked.connect([this]() {
		toggle_advanced_options_.toggle();
		update_advanced_options();
	});

	if (choose_map_ != nullptr) {
		choose_map_->sigclicked.connect([this]() { clicked_select_map(); });
	}
	if (choose_savegame_ != nullptr) {
		choose_savegame_->sigclicked.connect([this]() { clicked_select_savegame(); });
	}
	ok_.set_title(_("Start game"));

	lua_.reset(new LuaInterface());
	add_all_widgets();

	update_advanced_options();
	layout();

	initialization_complete();
}

LaunchGame::~LaunchGame() {
	if (ctrl_ != nullptr) {
		ctrl_->game_setup_aborted();
	}
}

void LaunchGame::add_all_widgets() {
	right_column_content_box_.add(&map_details_, UI::Box::Resizing::kExpandBoth);
	right_column_content_box_.add_space(1 * kPadding);
	right_column_content_box_.add(&warn_desyncing_addon_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add_space(4 * kPadding);
	right_column_content_box_.add(&configure_game_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	right_column_content_box_.add_space(3 * kPadding);
	right_column_content_box_.add(&win_condition_dropdown_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add_space(1 * kPadding);
	right_column_content_box_.add(&win_condition_duration_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add_space(3 * kPadding);
	right_column_content_box_.add(&toggle_advanced_options_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add_space(1 * kPadding);
	right_column_content_box_.add(&advanced_options_box_, UI::Box::Resizing::kFullSize);

	for (auto& pair : game_flag_checkboxes_) {
		advanced_options_box_.add(pair.second.first, UI::Box::Resizing::kFullSize);
		advanced_options_box_.add_space(1 * kPadding);
	}
	advanced_options_box_.add(&write_replay_, UI::Box::Resizing::kFullSize);

	if (choose_map_ != nullptr) {
		right_column_content_box_.add_space(3 * kPadding);
		right_column_content_box_.add(choose_map_, UI::Box::Resizing::kFullSize);
	}
	if (choose_savegame_ != nullptr) {
		right_column_content_box_.add_space(3 * kPadding);
		right_column_content_box_.add(choose_savegame_, UI::Box::Resizing::kFullSize);
	}
}

void LaunchGame::layout() {
	TwoColumnsFullNavigationMenu::layout();
	win_condition_dropdown_.set_desired_size(0, standard_height_);

	map_details_.set_max_size(0, right_column_box_.get_h() / 3);
	map_details_.force_new_dimensions(right_column_width_, standard_height_);
}

void LaunchGame::update_warn_desyncing_addon() {
	const bool has_desyncing_addon = std::any_of(
	   AddOns::g_addons.begin(), AddOns::g_addons.end(),
	   [](const AddOns::AddOnState& addon) { return addon.second && !addon.first->sync_safe; });

	warn_desyncing_addon_.set_visible(has_desyncing_addon);
	// TODO(Nordfriese): The scenario check is a quickfix for #5745 and related #4531,
	// remove when scenario replays are functional
	write_replay_.set_visible(!has_desyncing_addon && !settings_.settings().scenario);
}

bool LaunchGame::should_write_replay() const {
	return write_replay_.is_visible() && write_replay_.get_state();
}

void LaunchGame::update_advanced_options() {
	const bool show = toggle_advanced_options_.style() == UI::Button::VisualState::kPermpressed;
	advanced_options_box_.set_visible(show);
	toggle_advanced_options_.set_title(show ? _("Show fewer options") : _("Show more options"));
	layout();
}

void LaunchGame::update_peaceful_mode() {
	UI::Checkbox* checkbox = game_flag_checkboxes_.at(GameSettings::Flags::kPeaceful).first;
	bool forbidden =
	   peaceful_mode_forbidden_ || settings_.settings().scenario || settings_.settings().savegame;

	checkbox->set_enabled(!forbidden && settings_.can_change_map());
	if (forbidden) {
		checkbox->set_state(false);
	}

	if (settings_.settings().scenario) {
		checkbox->set_tooltip(_("The relations between players are set by the scenario"));
	} else if (settings_.settings().savegame) {
		checkbox->set_tooltip(_("The relations between players are set by the saved game"));
	} else if (peaceful_mode_forbidden_) {
		checkbox->set_tooltip(_("The selected win condition does not allow peaceful matches"));
	} else {
		checkbox->set_tooltip(_("Forbid fighting between players"));
	}
}

void LaunchGame::update_custom_starting_positions() {
	UI::Checkbox* checkbox =
	   game_flag_checkboxes_.at(GameSettings::Flags::kCustomStartingPositions).first;
	const GameSettings& settings = settings_.settings();
	const bool forbidden = settings.scenario || settings.savegame;

	if (forbidden || !settings_.can_change_map()) {
		checkbox->set_enabled(false);
		if (forbidden) {
			checkbox->set_state(false);
		}
	} else {
		bool allowed = false;
		for (const PlayerSettings& p : settings.players) {
			if (p.state != PlayerSettings::State::kClosed && p.state != PlayerSettings::State::kOpen &&
			    settings.get_tribeinfo(p.tribe)
			       .initializations[p.initialization_index]
			       .uses_map_starting_position) {
				allowed = true;
				break;
			}
		}
		checkbox->set_enabled(allowed);
		if (!allowed) {
			checkbox->set_state(false);
			checkbox->set_tooltip(
			   _("All selected starting conditions ignore the map’s starting positions"));
			return;
		}
	}

	if (settings_.settings().scenario) {
		checkbox->set_tooltip(_("The starting positions are set by the scenario"));
	} else if (settings_.settings().savegame) {
		checkbox->set_tooltip(_("The starting positions are set by the saved game"));
	} else {
		checkbox->set_tooltip(_(
		   "Allow the players to choose their own starting positions at the beginning of the game"));
	}
}

void LaunchGame::update_fogless() {
	UI::Checkbox* checkbox = game_flag_checkboxes_.at(GameSettings::Flags::kFogless).first;
	const GameSettings& settings = settings_.settings();
	const bool forbidden = settings.scenario || settings.savegame;

	if (forbidden || !settings_.can_change_map()) {
		checkbox->set_enabled(false);
		if (forbidden) {
			checkbox->set_state(false);
		}
	} else {
		checkbox->set_enabled(true);
	}

	if (settings_.settings().scenario) {
		checkbox->set_tooltip(_("Player vision is set by the scenario"));
	} else if (settings_.settings().savegame) {
		checkbox->set_tooltip(_("Player vision is set by the saved game"));
	} else {
		checkbox->set_tooltip(_("Give all players permanent vision of the entire map"));
	}
}

void LaunchGame::update_forbid_diplomacy() {
	UI::Checkbox* checkbox = game_flag_checkboxes_.at(GameSettings::Flags::kForbidDiplomacy).first;
	const GameSettings& settings = settings_.settings();
	const bool forbidden = settings.scenario || settings.savegame;

	if (forbidden || !settings_.can_change_map()) {
		checkbox->set_enabled(false);
		if (forbidden) {
			checkbox->set_state(false);
		}
	} else {
		checkbox->set_enabled(true);
	}

	if (settings_.settings().scenario) {
		checkbox->set_tooltip(_("Player relations are set by the scenario"));
	} else if (settings_.settings().savegame) {
		checkbox->set_tooltip(_("Player relations are set by the saved game"));
	} else {
		checkbox->set_tooltip(_("Disallow players to change their alliances during the game"));
	}
}

bool LaunchGame::init_win_condition_label() {
	win_condition_duration_.set_visible(false);
	if (settings_.settings().scenario) {
		win_condition_dropdown_.set_enabled(false);
		win_condition_dropdown_.set_label(_("Scenario"));
		win_condition_dropdown_.set_tooltip(_("Win condition is set through the scenario"));
		return true;
	}
	if (settings_.settings().savegame) {
		win_condition_dropdown_.set_enabled(false);
		/** Translators: This is a game type */
		win_condition_dropdown_.set_label(_("Saved Game"));
		win_condition_dropdown_.set_tooltip(
		   _("The game is a saved game – the win condition was set before."));
		return true;
	}
	win_condition_dropdown_.set_enabled(settings_.can_change_map());
	win_condition_dropdown_.set_label("");
	win_condition_dropdown_.set_tooltip("");
	return false;
}

/**
 * Fill the dropdown with the available win conditions.
 */
void LaunchGame::update_win_conditions() {
	if (!init_win_condition_label()) {
		std::set<std::string> tags;
		if (!settings_.settings().mapfilename.empty()) {
			Widelands::Map map;
			std::unique_ptr<Widelands::MapLoader> ml =
			   map.get_correct_loader(settings_.settings().mapfilename);
			if (ml != nullptr) {
				ml->preload_map(true, nullptr);
				tags = map.get_tags();
			}
		}
		load_win_conditions(tags);
	}
}

void LaunchGame::load_win_conditions(const std::set<std::string>& tags) {
	win_condition_dropdown_.clear();
	try {
		// Make sure that the last win condition is still valid. If not, pick the first one
		// available.
		if (last_win_condition_.empty()) {
			last_win_condition_ = settings_.settings().win_condition_scripts.front();
		}
		std::unique_ptr<LuaTable> t = win_condition_if_valid(last_win_condition_, tags);
		for (const std::string& win_condition_script : settings_.settings().win_condition_scripts) {
			if (t) {
				break;
			}
			last_win_condition_ = win_condition_script;
			t = win_condition_if_valid(last_win_condition_, tags);
		}

		// Now fill the dropdown.
		for (const std::string& win_condition_script : settings_.settings().win_condition_scripts) {
			try {
				t = win_condition_if_valid(win_condition_script, tags);
				if (t) {
					std::string name;
					std::string desc;
					// Prevent propagation of the textdomain
					if (t->has_key("textdomain")) {
						std::unique_ptr<i18n::GenericTextdomain> td(
						   AddOns::create_textdomain_for_addon(t->get_string("textdomain")));
						name = _(t->get_string("name"));
						desc = t->get_string("description");
					} else {
						i18n::Textdomain td("win_conditions");
						name = _(t->get_string("name"));
						desc = t->get_string("description");
					}
					win_condition_dropdown_.add(name, win_condition_script, nullptr,
					                            win_condition_script == last_win_condition_, desc);
				}
			} catch (LuaTableKeyError& e) {
				log_err("Launch Game: Error loading win condition: %s %s\n",
				        win_condition_script.c_str(), e.what());
			}
		}
	} catch (const std::exception& e) {
		const std::string error_message =
		   format(_("Unable to determine valid win conditions because the map ‘%s’ "
		            "could not be loaded."),
		          settings_.settings().mapfilename);
		win_condition_dropdown_.set_errored(error_message);
		log_err("Launch Game: Exception: %s %s\n", error_message.c_str(), e.what());
	}
}

std::unique_ptr<LuaTable>
LaunchGame::win_condition_if_valid(const std::string& win_condition_script,
                                   const std::set<std::string>& tags) const {
	bool is_usable = true;
	std::unique_ptr<LuaTable> t;
	try {
		t = lua_->run_script(win_condition_script);
		t->do_not_warn_about_unaccessed_keys();

		// Skip this win condition if the map doesn't have all the required tags
		if (t->has_key("map_tags")) {
			for (const std::string& map_tag : t->get_table("map_tags")->array_entries<std::string>()) {
				if (tags.count(map_tag) == 0u) {
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

void LaunchGame::win_condition_duration_changed() {
	settings_.set_win_condition_duration(win_condition_duration_.get_value());
}
}  // namespace FsMenu
