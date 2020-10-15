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

#include "logic/suggested_teams.h"

#include <cassert>

#include "graphic/playercolor.h"

namespace Widelands {

std::string SuggestedTeamLineup::as_richtext() const {
	std::string result = "";
	bool is_first = true;
	for (const Widelands::SuggestedTeam& team : *this) {
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


}  // namespace Widelands

