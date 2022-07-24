/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "editor/tools/set_starting_pos_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"

// global variable to pass data from callback to class
static int32_t current_player_;

namespace {

Widelands::NodeCaps set_starting_pos_tool_nodecaps(const Widelands::FCoords& c,
                                                   const Widelands::Map& map) {
	// Area around already placed players
	Widelands::PlayerNumber const nr_players = map.get_nrplayers();
	for (Widelands::PlayerNumber p = 1, last = current_player_ - 1;; ++p) {
		for (; p <= last; ++p) {
			if (Widelands::Coords const sp = map.get_starting_pos(p)) {
				if (map.calc_distance(sp, c) < Widelands::kMinSpaceAroundPlayers) {
					return Widelands::NodeCaps::CAPS_NONE;
				}
			}
		}
		if (last == nr_players) {
			break;
		}
		last = nr_players;
	}

	Widelands::NodeCaps const caps = c.field->nodecaps();
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG) {
		return caps;
	}
	return Widelands::NodeCaps::CAPS_NONE;
}

}  // namespace

EditorSetStartingPosTool::EditorSetStartingPosTool(EditorInteractive& parent)
   : EditorTool(parent, *this, *this, false) {
	current_player_ = 1;
}

int32_t EditorSetStartingPosTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                    EditorActionArgs* /* args */,
                                                    Widelands::Map* map) {
	assert(0 <= center.node.x);
	assert(center.node.x < map->get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map->get_height());
	if (current_player_ != 0) {
		if (map->get_nrplayers() < current_player_) {
			//  Mmh, my current player is not valid. Maybe the user has loaded a
			//  new map while this tool was active. We set the new player to a
			//  valid one. The sel pointer is the only thing that stays wrong, but
			//  this is not important
			current_player_ = 1;
		}

		//  check if field is valid
		if (set_starting_pos_tool_nodecaps(map->get_fcoords(center.node), *map) !=
		    Widelands::NodeCaps::CAPS_NONE) {
			map->set_starting_pos(current_player_, center.node);
		}
	}
	return 1;
}

Widelands::NodeCaps
EditorSetStartingPosTool::nodecaps_for_buildhelp(const Widelands::FCoords& fcoords,
                                                 const Widelands::EditorGameBase& egbase) {
	return set_starting_pos_tool_nodecaps(fcoords, egbase.map());
}

Widelands::PlayerNumber EditorSetStartingPosTool::get_current_player() const {
	return current_player_;
}

void EditorSetStartingPosTool::set_current_player(int32_t const i) {
	current_player_ = i;
}
