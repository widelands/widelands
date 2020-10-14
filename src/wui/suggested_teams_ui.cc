/*
 * Copyright (C) 2015-2019 by the Widelands Development Team
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

#include "wui/suggested_teams_ui.h"

#include "base/i18n.h"
#include "graphic/playercolor.h"

std::string format_suggested_teams_lineup(const Widelands::SuggestedTeamLineup& lineup) {
	std::string result = "";
	bool is_first = true;
	for (const Widelands::SuggestedTeam& team : lineup) {
		if (!is_first) {
			result += " x ";
		}
		is_first = false;

		for (Widelands::PlayerNumber player : team) {
			assert(player < kMaxPlayers);
			result += "<img src=images/players/player_position_menu.png color=" + kPlayerColors[player].hex_value() + ">";
		}
	}
	return result;
}

SuggestedTeamsDropdown::SuggestedTeamsDropdown(UI::Panel* parent, int32_t x, int32_t y, uint32_t list_w, int button_dimension) :
	UI::Dropdown<size_t>(parent, "suggested_teams_dropdown",
	         x,
	         y,
	         list_w,
	         10,
	         button_dimension,
	         "",
	         UI::DropdownType::kTextual,
	         UI::PanelStyle::kFsMenu,
	         UI::ButtonStyle::kFsMenuMenu, true) {
	set_visible(false);
	set_enabled(false);
}

void SuggestedTeamsDropdown::rebuild(const std::vector<Widelands::SuggestedTeamLineup>& suggested_teams, bool can_change_map) {
	set_visible(false);
	set_enabled(false);
	clear();
	suggested_teams_ = suggested_teams;
	if (!suggested_teams_.empty()) {
		add(_("No Teams"), Widelands::kNoSuggestedTeam, nullptr, true);

		for (size_t i = 0; i < suggested_teams_.size(); ++i) {
			add(format_suggested_teams_lineup(suggested_teams_.at(i)), i);
		}
		set_enabled(can_change_map);
	} else {
		add(_("No Teams"), Widelands::kNoSuggestedTeam, nullptr, true);
	}
	set_visible(true);
}

const Widelands::SuggestedTeamLineup* SuggestedTeamsDropdown::get_lineup(size_t index) const {
	if (index == Widelands::kNoSuggestedTeam) {
		return nullptr;
	} else {
		assert(index < suggested_teams_.size());
		return (&suggested_teams_.at(index));
	}
}
