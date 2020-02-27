/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_SET_STARTING_POS_TOOL_H
#define WL_EDITOR_TOOLS_SET_STARTING_POS_TOOL_H

#include <vector>

#include "editor/tools/tool.h"

// How much place should be left around a player position
// where no other player can start
#define MIN_PLACE_AROUND_PLAYERS 24

/// Sets the starting position of players.
struct EditorSetStartingPosTool : public EditorTool {
	EditorSetStartingPosTool();

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>&,
	                          EditorInteractive&,
	                          EditorActionArgs*,
	                          Widelands::Map*) override;
	const Image* get_sel_impl() const override {
		return playercolor_image(get_current_player() - 1, "images/players/player_position_menu.png");
	}

	Widelands::PlayerNumber get_current_player() const;
	void set_current_player(int32_t);
	bool has_size_one() const override {
		return true;
	}
	Widelands::NodeCaps nodecaps_for_buildhelp(const Widelands::FCoords& fcoords,
	                                           const Widelands::EditorGameBase&) override;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_SET_STARTING_POS_TOOL_H
