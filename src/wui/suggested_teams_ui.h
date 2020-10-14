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

#ifndef WL_WUI_SUGGESTED_TEAMS_UI_H
#define WL_WUI_SUGGESTED_TEAMS_UI_H

#include <string>
#include <vector>

#include "logic/widelands.h"
#include "ui_basic/dropdown.h"

/// Format a teams lineup using player icons and <img> tags
std::string format_suggested_teams_lineup(const Widelands::SuggestedTeamLineup& lineup);

/// Lists available team lineups for selection
class SuggestedTeamsDropdown : public UI::Dropdown<size_t> {
public:
	explicit SuggestedTeamsDropdown(UI::Panel* parent,
									int32_t x,
									int32_t y,
									uint32_t list_w,
									int button_dimension);
	void rebuild(const std::vector<Widelands::SuggestedTeamLineup>& suggested_teams, bool can_change_map);
	const Widelands::SuggestedTeamLineup* get_lineup(size_t index) const;

private:
	std::vector<Widelands::SuggestedTeamLineup> suggested_teams_;
};

#endif  // end of include guard: WL_WUI_SUGGESTED_TEAMS_UI_H
