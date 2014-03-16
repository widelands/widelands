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

#ifndef EDITOR_SET_STARTING_POS_TOOL_H
#define EDITOR_SET_STARTING_POS_TOOL_H

#include "editor/tools/editor_tool.h"
#include "logic/widelands.h"

// How much place should be left around a player position
// where no other player can start
#define MIN_PLACE_AROUND_PLAYERS 24
#define STARTING_POS_HOTSPOT_Y 55

#define FSEL_PIC_FILENAME "pics/fsel_editor_set_player_00_pos.png"

/// Sets the starting position of players.
struct Editor_Set_Starting_Pos_Tool : public Editor_Tool {
	Editor_Set_Starting_Pos_Tool();

	int32_t handle_click_impl
		(Widelands::Map &, Widelands::Node_and_Triangle<>,
		 Editor_Interactive &, Editor_Action_Args &) override;
	char const * get_sel_impl() const override
		{return m_current_sel_pic;}

	Widelands::Player_Number get_current_player() const;
	void set_current_player(int32_t);
	bool has_size_one() const override {return true;}

private:
	char fsel_picsname[sizeof(FSEL_PIC_FILENAME)];
	char const * m_current_sel_pic;
};

int32_t Editor_Tool_Set_Starting_Pos_Callback
	(const Widelands::TCoords<Widelands::FCoords>& c, Widelands::Map& map);

#endif
