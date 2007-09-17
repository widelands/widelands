/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
#include "graphic.h"
#include "map.h"
#include "overlay_manager.h"


// global variable to pass data from callback to class
static int m_current_player;

/*
 * static callback function for overlay calculation
 */
int Editor_Tool_Set_Starting_Pos_Callback
(const TCoords<FCoords> c, void * data, int)
{
	const Map & map = *static_cast<const Map *>(data);

	// Area around already placed players
	const Player_Number nr_players = map.get_nrplayers();
	for (Player_Number p = 1, last = m_current_player - 1;; ++p) {
		for (; p <= last; ++p)
			if (const Coords sp = map.get_starting_pos(p))
				if (map.calc_distance(sp, c) < MIN_PLACE_AROUND_PLAYERS) return 0;
		if (last == nr_players) break;
		last = nr_players;
	}

	const int caps = c.field->get_caps();
   if ((caps&BUILDCAPS_SIZEMASK)==BUILDCAPS_BIG)
      return caps;


   return 0;
}


Editor_Set_Starting_Pos_Tool::Editor_Set_Starting_Pos_Tool()
: Editor_Tool(*this, *this), m_current_sel_pic("")
{m_current_player = 0;}


int Editor_Set_Starting_Pos_Tool::handle_click_impl
(Map & map, const Node_and_Triangle<> center, Editor_Interactive & parent)
{
	assert(0 <= center.node.x);
	assert(center.node.x < map.get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map.get_height());
	if (m_current_player) {
		if (map.get_nrplayers() < m_current_player) {
         // mmh, my current player is not valid
         // maybe the user has loaded a new map while this tool
         // was activated. we set the new player to a valid one. The
			// sel pointer is the only thing that stays wrong, but this is not
			// important
         m_current_player=1;
		}

		const Coords old_sp = map.get_starting_pos(m_current_player);

      // If the player is already created in the editor, this means
      // that there might be already a hq placed somewhere. This needs to be
      // deleted before a starting position change can occure
		if
			(parent.editor().get_player(m_current_player)
			 and
			 old_sp
			 and
			 dynamic_cast<const Building *>(map[old_sp].get_immovable()))
			return 1;

		char picname[] = "pics/editor_player_??_starting_pos.png";
		picname[19] = static_cast<char>(m_current_player / 10 + 0x30);
		picname[20] = static_cast<char>(m_current_player % 10 + 0x30);
		const uint picid = g_gr->get_picture(PicMod_Game,  picname);
		uint w, h;
		g_gr->get_picture_size(picid, w, h);

      // check if field is valid
		if (Editor_Tool_Set_Starting_Pos_Callback
		    (map.get_fcoords(center.node), &map, 0))
		{
			Overlay_Manager & overlay_manager = map.overlay_manager();
         // Remove old overlay if any
			overlay_manager.remove_overlay(old_sp, picid);

         // Add new overlay
			overlay_manager.register_overlay
				(center.node, picid, 8, Point(w / 2, STARTING_POS_HOTSPOT_Y));

         // And set new player pos
			map.set_starting_pos(m_current_player, center.node);

		}
	}
   return 1;
}

Player_Number Editor_Set_Starting_Pos_Tool::get_current_player() const throw()
{return m_current_player;}

void Editor_Set_Starting_Pos_Tool::set_current_player(int i) {
      m_current_player=i;

      std::string picsname="pics/fsel_editor_set_player_";
      picsname+=static_cast<char>((m_current_player/10) + 0x30);
      picsname+=static_cast<char>((m_current_player%10) + 0x30);
      picsname+="_pos.png";
      m_current_sel_pic = picsname;
}
