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

#include "ui_fsmenu/singleplayersetupbox.h"

#include <memory>

#include "base/i18n.h"
#include "graphic/playercolor.h"
#include "graphic/style_manager.h"
#include "logic/game.h"
#include "logic/player.h"
#include "map_io/map_loader.h"

SinglePlayerActivePlayerGroup::SinglePlayerActivePlayerGroup(UI::Panel* const parent,
                                                             int32_t const,
                                                             int32_t const h,
                                                             PlayerSlot id,
                                                             GameSettingsProvider* const settings)
   : UI::Box(parent, 0, 0, UI::Box::Horizontal),
     id_(id),
     settings_(settings),
     player_(this,
             "player",
             0,
             0,
             h,
             h,
             UI::ButtonStyle::kFsMenuSecondary,
             playercolor_image(id, "images/players/player_position_menu.png"),
             (boost::format(_("Player %u")) % static_cast<unsigned>(id_ + 1)).str(),
             UI::Button::VisualState::kFlat),
     player_type_(this,
                  (boost::format("dropdown_type%d") % static_cast<unsigned>(id)).str(),
                  0,
                  0,
                  h,
                  h,
                  settings,
                  id),
     tribe_(this,
            (boost::format("dropdown_tribe%d") % static_cast<unsigned>(id)).str(),
            0,
            0,
            h,
            h,
            settings,
            id),
     start_type(this,
                (boost::format("dropdown_init%d") % static_cast<unsigned>(id)).str(),
                0,
                0,
                8 * h,
                h,
                settings,
                id),
     teams_(this,
            (boost::format("dropdown_team%d") % static_cast<unsigned>(id)).str(),
            0,
            0,
            h,
            h,
            settings,
            id) {

	add(&player_);
	add(player_type_.get_dropdown());
	add(tribe_.get_dropdown());
	add(start_type.get_dropdown(), UI::Box::Resizing::kExpandBoth);
	add(teams_.get_dropdown());

	player_.set_disable_style(UI::ButtonDisableStyle::kFlat);
	player_.set_enabled(false);
	update();
}
void SinglePlayerActivePlayerGroup::force_new_dimensions(float, uint32_t standard_element_height) {
	player_.set_desired_size(standard_element_height, standard_element_height);
	player_type_.set_desired_size(standard_element_height, standard_element_height);
	tribe_.set_desired_size(standard_element_height, standard_element_height);
	start_type.set_desired_size(8 * standard_element_height, standard_element_height);
	teams_.set_desired_size(standard_element_height, standard_element_height);
}

void SinglePlayerActivePlayerGroup::update() {
	const GameSettings& settings = settings_->settings();

	player_type_.rebuild();

	const PlayerSettings& player_setting = settings.players[id_];
	player_.set_tooltip(player_setting.name.empty() ? "" : player_setting.name);
	if (player_setting.state == PlayerSettings::State::kClosed ||
	    player_setting.state == PlayerSettings::State::kOpen) {

		teams_.set_visible(false);
		teams_.set_enabled(false);

		tribe_.set_visible(false);
		tribe_.set_enabled(false);

		start_type.set_visible(false);
		start_type.set_enabled(false);
	} else {  // kHuman, kShared, kComputer
		tribe_.rebuild();
		start_type.rebuild();
		teams_.rebuild();
	}
}

SinglePlayerSetupBox::SinglePlayerSetupBox(UI::Panel* const parent,
                                           GameSettingsProvider* const settings,
                                           uint32_t standard_element_height,
                                           uint32_t padding)
   : UI::Box(parent, 0, 0, UI::Box::Vertical),
     settings_(settings),
     standard_height(standard_element_height),
     scrollable_playerbox(this, 0, 0, UI::Box::Vertical),
     title_(this,
            0,
            0,
            0,
            0,
            _("Players"),
            UI::Align::kRight,
            g_style_manager->font_style(UI::FontStyle::kFsGameSetupHeadings)),
	 suggested_teams_dropdown_(this,
							   0,
							   0,
							   0,
							   0),
		  selected_lineup_(nullptr),
		  suggested_team_selection_in_progress_(false) {
	add(&title_, Resizing::kAlign, UI::Align::kCenter);
	add(&suggested_teams_dropdown_, UI::Box::Resizing::kFullSize);
	add_space(3 * padding);
	add(&scrollable_playerbox, Resizing::kExpandBoth);
	scrollable_playerbox.set_scrolling(true);

	suggested_teams_dropdown_.selected.connect([this] { select_teams(); });

	subscriber_ = Notifications::subscribe<NoteGameSettings>([this](const NoteGameSettings& n) {
		switch (n.action) {
		case NoteGameSettings::Action::kTeam:
			if (!suggested_team_selection_in_progress_) {
				check_teams();
				update();
			}
			break;
		case NoteGameSettings::Action::kMap:
			reset();
			break;
		default:
			update();
			check_teams();
		}
	});
}

void SinglePlayerSetupBox::update() {

	const GameSettings& settings = settings_->settings();
	const size_t number_of_players = settings.players.size();

	for (PlayerSlot i = active_player_groups.size(); i < number_of_players; ++i) {
		active_player_groups.push_back(new SinglePlayerActivePlayerGroup(
		   &scrollable_playerbox, 0, standard_height, i, settings_));
		scrollable_playerbox.add(active_player_groups.at(i), Resizing::kFullSize);
	}

	for (auto& p : active_player_groups) {
		p->update();
	}
}

void SinglePlayerSetupBox::force_new_dimensions(float scale, uint32_t standard_element_height) {
	standard_height = standard_element_height;
	title_.set_font_scale(scale);
	for (auto& active_player_group : active_player_groups) {
		active_player_group->force_new_dimensions(scale, standard_element_height);
	}
	suggested_teams_dropdown_.set_desired_size(suggested_teams_dropdown_.get_w(), standard_element_height);
}
void SinglePlayerSetupBox::reset() {
	for (auto& p : active_player_groups) {
		p->die();
	}
	active_player_groups.clear();
	update();
}

void SinglePlayerSetupBox::reset_teams(const Widelands::Map& map) {
	suggested_team_selection_in_progress_ = true;
	selected_lineup_ = nullptr;
	suggested_teams_dropdown_.rebuild(map.get_suggested_teams(), settings_->can_change_map());

	if (settings_->can_change_map()) {
		// Reset teams and slot state
		for (size_t i = 0; i < settings_->settings().players.size(); ++i) {
			settings_->set_player_team(i, 0);
			if (settings_->settings().players.at(i).state == PlayerSettings::State::kClosed) {
				settings_->set_player_state(i, PlayerSettings::State::kOpen);
			}
		}

		// If it is a scenario, auto-set the teams if there is only 1
		if (settings_->settings().scenario && map.get_suggested_teams().size() == 1) {
			suggested_teams_dropdown_.select(0);
			select_teams();
			suggested_teams_dropdown_.set_enabled(false);
		}
	}
	suggested_team_selection_in_progress_ = false;
}

void SinglePlayerSetupBox::select_teams() {
	if (suggested_team_selection_in_progress_) {
		return;
	}

	suggested_team_selection_in_progress_ = true;
	const size_t sel = suggested_teams_dropdown_.get_selected();
	selected_lineup_ = suggested_teams_dropdown_.get_lineup(sel);

	std::vector<uint8_t> teams_to_set(settings_->settings().players.size(), 0);

	if (selected_lineup_ != nullptr) {
		for (size_t i = 0; i < selected_lineup_->size(); ++i) {
			for (PlayerSlot pl : selected_lineup_->at(i)) {
				teams_to_set.at(pl) = i + 1;
			}
		}
	}

	for (size_t i = 0; i < teams_to_set.size(); ++i) {
		uint8_t new_team = teams_to_set.at(i);
		// Set team if it has changed
		if (new_team != settings_->settings().players.at(i).team) {
			settings_->set_player_team(i, new_team);
			active_player_groups.at(i)->update();
		}
		// Automatically open/close slots according to selected teams
		if (sel != Widelands::kNoSuggestedTeam && new_team == 0) {
			settings_->set_player_state(i, PlayerSettings::State::kClosed);
		} else if (settings_->settings().players.at(i).state == PlayerSettings::State::kClosed) {
			settings_->set_player_state(i, PlayerSettings::State::kOpen);
		}
	}
	suggested_team_selection_in_progress_ = false;
}

void SinglePlayerSetupBox::check_teams() {
	// NOCOM When player changes slot, they take their team with them. Keep or change this behavior?
	if (suggested_team_selection_in_progress_ || selected_lineup_ == nullptr) {
		return;
	}

	std::vector<uint8_t> dropdown_teams(settings_->settings().players.size(), 0);
	for (size_t i = 0; i < selected_lineup_->size(); ++i) {
		for (PlayerSlot pl : selected_lineup_->at(i)) {
			dropdown_teams.at(pl) = i + 1;
		}
	}

	for (size_t i = 0; i < dropdown_teams.size(); ++i) {
		uint8_t dropdown_team = dropdown_teams.at(i);
		// Reset if team has changed
		if (dropdown_team != settings_->settings().players.at(i).team) {
			suggested_teams_dropdown_.select(Widelands::kNoSuggestedTeam);
			selected_lineup_ = nullptr;
			return;
		}
	}
}

void SinglePlayerSetupBox::update_team(PlayerSlot pos) {
	if (selected_lineup_ != nullptr && pos < settings_->settings().players.size()) {
		assert(suggested_teams_dropdown_.is_visible());
		for (size_t i = 0; i < selected_lineup_->size(); ++i) {
			for (PlayerSlot pl : selected_lineup_->at(i)) {
				if (pl == pos) {
					settings_->set_player_team(pos, i + 1);
					break;
				}
			}
		}
	}
}
