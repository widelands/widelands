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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "editor_set_starting_pos_tool.h"

#include "building.h"
#include "editorinteractive.h"
#include "editor_tool.h"
#include "graphic/graphic.h"
#include "map.h"
#include "overlay_manager.h"

// global variable to pass data from callback to class
static int32_t m_current_player;

/*
 * static callback function for overlay calculation
 */
int32_t Editor_Tool_Set_Starting_Pos_Callback
	(Widelands::TCoords<Widelands::FCoords> const c, void * data, int32_t)
{
	Widelands::Map const & map = *static_cast<Widelands::Map const *>(data);

	// Area around already placed players
	Widelands::Player_Number const nr_players = map.get_nrplayers();
	for (Widelands::Player_Number p = 1, last = m_current_player - 1;; ++p) {
		for (; p <= last; ++p)
			if (Widelands::Coords const sp = map.get_starting_pos(p))
				if (map.calc_distance(sp, c) < MIN_PLACE_AROUND_PLAYERS) return 0;
		if (last == nr_players) break;
		last = nr_players;
	}

	const int32_t caps = c.field->get_caps();
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG)
		return caps;


	return 0;
}


Editor_Set_Starting_Pos_Tool::Editor_Set_Starting_Pos_Tool()
: Editor_Tool(*this, *this), m_current_sel_pic(0)
{
	m_current_player = 0;
	strcpy(fsel_picsname, FSEL_PIC_FILENAME);
}


int32_t Editor_Set_Starting_Pos_Tool::handle_click_impl
	(Widelands::Map               &       map,
	 Widelands::Node_and_Triangle<> const center,
	 Editor_Interactive           &       parent)
{
	assert(0 <= center.node.x);
	assert(center.node.x < map.get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map.get_height());
	if (m_current_player) {
		if (map.get_nrplayers() < m_current_player) {
			//  Mmh, my current player is not valid. Maybe the user has loaded a
			//  new map while this tool was active. We set the new player to a
			//  valid one. The sel pointer is the only thing that stays wrong, but
			//  this is not important
			m_current_player = 1;
		}

		Widelands::Coords const old_sp = map.get_starting_pos(m_current_player);

		//  If the player is already created in the editor, this means that there
		//  might be already a hq placed somewhere. This needs to be deleted
		//  before a starting position change can occur.
		if
			(parent.egbase().get_player(m_current_player)
			 and
			 old_sp
			 and
			 dynamic_cast<const Widelands::Building *>
			 	(map[old_sp].get_immovable()))
			return 1;

		char picname[] = "pics/editor_player_00_starting_pos.png";
		picname[19] += m_current_player / 10;
		picname[20] += m_current_player % 10;
		const uint32_t picid = g_gr->get_picture(PicMod_Game,  picname);
		uint32_t w, h;
		g_gr->get_picture_size(picid, w, h);

		//  check if field is valid
		if
			(Editor_Tool_Set_Starting_Pos_Callback
			 	(map.get_fcoords(center.node), &map, 0))
		{
			Overlay_Manager & overlay_manager = map.overlay_manager();
			//  remove old overlay if any
			overlay_manager.remove_overlay(old_sp, picid);

			//  add new overlay
			overlay_manager.register_overlay
				(center.node, picid, 8, Point(w / 2, STARTING_POS_HOTSPOT_Y));

			//  set new player pos
			map.set_starting_pos(m_current_player, center.node);

		}
	}
	return 1;
}

Widelands::Player_Number Editor_Set_Starting_Pos_Tool::get_current_player
	() const
throw ()
{return m_current_player;}


void Editor_Set_Starting_Pos_Tool::set_current_player(int32_t const i) {
	m_current_player = i;

	fsel_picsname[28] = '0' + m_current_player / 10;
	fsel_picsname[29] = '0' + m_current_player % 10;
	m_current_sel_pic = fsel_picsname;
}
