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

#include "ui_fsmenu/playersetupbox.h"

#include <memory>
#include <vector>

#include "base/i18n.h"
#include "logic/map.h"

SuggestedTeamsDropdown::SuggestedTeamsDropdown(
   UI::Panel* parent, int32_t x, int32_t y, uint32_t list_w, int button_dimension)
   : UI::Dropdown<size_t>(parent,
                          "suggested_teams_dropdown",
                          x,
                          y,
                          list_w,
                          10,
                          button_dimension,
                          "",
                          UI::DropdownType::kTextual,
                          UI::PanelStyle::kFsMenu,
                          UI::ButtonStyle::kFsMenuMenu,
                          true) {
	set_visible(false);
	set_enabled(false);
}

void SuggestedTeamsDropdown::rebuild(
   const std::vector<Widelands::SuggestedTeamLineup>& suggested_teams, bool can_change_map) {
	set_visible(false);
	set_enabled(false);
	clear();
	suggested_teams_ = suggested_teams;
	if (!suggested_teams_.empty()) {
		add(_("Suggested Teams"), Widelands::SuggestedTeamLineup::none(), nullptr, true);

		for (size_t i = 0; i < suggested_teams_.size(); ++i) {
			add(suggested_teams_.at(i).as_richtext(), i);
		}
		set_enabled(can_change_map);
	} else {
		add(_("No Suggested Teams"), Widelands::SuggestedTeamLineup::none(), nullptr, true);
	}
	set_visible(true);
}

const Widelands::SuggestedTeamLineup* SuggestedTeamsDropdown::get_lineup(size_t index) const {
	if (index == Widelands::SuggestedTeamLineup::none()) {
		return nullptr;
	} else {
		assert(index < suggested_teams_.size());
		return (&suggested_teams_.at(index));
	}
}

PlayerSetupBox::PlayerSetupBox(UI::Panel* const parent,
                               GameSettingsProvider* const settings,
                               uint32_t standard_element_height,
                               uint32_t padding)
   : UI::Box(parent, 0, 0, UI::Box::Vertical, 0, 0, padding),
     standard_height_(standard_element_height),
     settings_(settings),
     scrollable_playerbox_(this, 0, 0, UI::Box::Vertical),
     title_(this,
            0,
            0,
            0,
            0,
            _("Players"),
            UI::Align::kRight,
            g_style_manager->font_style(UI::FontStyle::kFsGameSetupHeadings)),
     suggested_teams_dropdown_(this, 0, 0, 0, 0),
     selected_lineup_(nullptr),
     suggested_team_selection_in_progress_(false) {
	add(&title_, Resizing::kAlign, UI::Align::kCenter);
	add_space(padding);
	add(&suggested_teams_dropdown_, UI::Box::Resizing::kFullSize);
	add_space(2 * padding);
	add(&scrollable_playerbox_, Resizing::kExpandBoth);
	scrollable_playerbox_.set_scrolling(true);

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

void PlayerSetupBox::reset_teams(const std::vector<Widelands::SuggestedTeamLineup>& suggested_teams) {
	suggested_team_selection_in_progress_ = true;
	selected_lineup_ = nullptr;
	suggested_teams_dropdown_.rebuild(suggested_teams, settings_->can_change_map());

	if (settings_->can_change_map()) {
		// Reset teams and slot state
		for (size_t i = 0; i < settings_->settings().players.size(); ++i) {
			settings_->set_player_team(i, 0);
			if (settings_->settings().players.at(i).state == PlayerSettings::State::kClosed) {
				settings_->set_player_state(i, PlayerSettings::State::kOpen);
			}
		}

		// If it is a scenario, auto-set the teams if there is only 1
		if (settings_->settings().scenario && suggested_teams.size() == 1) {
			suggested_teams_dropdown_.select(0);
			select_teams();
			suggested_teams_dropdown_.set_enabled(false);
		}
	}
	suggested_team_selection_in_progress_ = false;
}

void PlayerSetupBox::select_teams() {
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
			update_player_group(i);
		}
		// Automatically open/close slots according to selected teams
		if (sel != Widelands::SuggestedTeamLineup::none() && new_team == 0) {
			settings_->set_player_state(i, PlayerSettings::State::kClosed);
		} else if (settings_->settings().players.at(i).state == PlayerSettings::State::kClosed) {
			settings_->set_player_state(i, PlayerSettings::State::kOpen);
		}
	}
	suggested_team_selection_in_progress_ = false;
}

void PlayerSetupBox::check_teams() {
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
			suggested_teams_dropdown_.select(Widelands::SuggestedTeamLineup::none());
			selected_lineup_ = nullptr;
			return;
		}
	}
}
