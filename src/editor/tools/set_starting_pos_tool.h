/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "editor/tools/tool.h"
#include "logic/widelands.h"

// How much place should be left around a player position
// where no other player can start
#define MIN_PLACE_AROUND_PLAYERS 24
#define STARTING_POS_HOTSPOT_Y 55

/// Sets the starting position of players.
struct EditorSetStartingPosTool : public EditorTool {
	EditorSetStartingPosTool();

	int32_t handle_click_impl(const Widelands::World& world,
	                          Widelands::NodeAndTriangle<>,
	                          EditorInteractive&,
	                          EditorActionArgs*,
							  Widelands::Map*) override;
	char const * get_sel_impl() const override
		{return current_sel_pic_;}

	Widelands::PlayerNumber get_current_player() const;
	void set_current_player(int32_t);
	bool has_size_one() const override {return true;}

private:
	char const * fsel_picsname_;
	char const * current_sel_pic_;
};

int32_t editor_tool_set_starting_pos_callback
	(const Widelands::TCoords<Widelands::FCoords>& c, Widelands::Map& map);

#endif  // end of include guard: WL_EDITOR_TOOLS_SET_STARTING_POS_TOOL_H
