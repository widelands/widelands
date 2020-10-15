/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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

#ifndef WL_LOGIC_SUGGESTED_TEAMS_H
#define WL_LOGIC_SUGGESTED_TEAMS_H

#include <string>
#include <vector>

#include "logic/widelands.h"

namespace Widelands {

// For suggested teams info during map preload
using SuggestedTeam = std::vector<PlayerNumber>;  // Players in a team

// Recommended teams to play against each other
struct SuggestedTeamLineup : std::vector<SuggestedTeam> {
	static size_t none() {
		return std::numeric_limits<size_t>::max();
	}

	/// Format this teams lineup using player icons and <img> tags
	std::string as_richtext() const;
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_SUGGESTED_TEAMS_H
