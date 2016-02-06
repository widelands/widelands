/*
 * Copyright (C) 2002-2004, 2006-2012 by the Widelands Development Team
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

#include "editor/tools/editor_set_starting_pos_tool.h"

#include "editor/editorinteractive.h"
#include "editor/tools/editor_tool.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "wui/field_overlay_manager.h"

namespace {
static char const * const player_pictures[] = {
	"images/players/editor_player_01_starting_pos.png",
	"images/players/editor_player_02_starting_pos.png",
	"images/players/editor_player_03_starting_pos.png",
	"images/players/editor_player_04_starting_pos.png",
	"images/players/editor_player_05_starting_pos.png",
	"images/players/editor_player_06_starting_pos.png",
	"images/players/editor_player_07_starting_pos.png",
	"images/players/editor_player_08_starting_pos.png"
};
static char const * const player_pictures_small[] = {
	"images/players/fsel_editor_set_player_01_pos.png",
	"images/players/fsel_editor_set_player_02_pos.png",
	"images/players/fsel_editor_set_player_03_pos.png",
	"images/players/fsel_editor_set_player_04_pos.png",
	"images/players/fsel_editor_set_player_05_pos.png",
	"images/players/fsel_editor_set_player_06_pos.png",
	"images/players/fsel_editor_set_player_07_pos.png",
	"images/players/fsel_editor_set_player_08_pos.png"
};
} // namespace

// global variable to pass data from callback to class
static int32_t m_current_player;

/*
 * static callback function for overlay calculation
 */
int32_t editor_tool_set_starting_pos_callback
	(const Widelands::TCoords<Widelands::FCoords>& c, Widelands::Map& map)
{
	// Area around already placed players
	Widelands::PlayerNumber const nr_players = map.get_nrplayers();
	for (Widelands::PlayerNumber p = 1, last = m_current_player - 1;; ++p) {
		for (; p <= last; ++p)
			if (Widelands::Coords const sp = map.get_starting_pos(p))
				if (map.calc_distance(sp, c) < MIN_PLACE_AROUND_PLAYERS)
					return 0;
		if (last == nr_players)
			break;
		last = nr_players;
	}

	Widelands::NodeCaps const caps = c.field->nodecaps();
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG)
		return caps;


	return 0;
}

EditorSetStartingPosTool::EditorSetStartingPosTool()
	: EditorTool(*this, *this, false), m_current_sel_pic(nullptr)
{
	m_current_player = 0;
	fsel_picsname = "images/players/fsel_editor_set_player_01_pos.png";
}

int32_t EditorSetStartingPosTool::handle_click_impl(const Widelands::World&,
                                                    Widelands::NodeAndTriangle<> const center,
                                                    EditorInteractive& eia,
                                                    EditorActionArgs*,
													Widelands::Map* map) {
	assert(0 <= center.node.x);
	assert(center.node.x < map->get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map->get_height());
	if (m_current_player) {
		if (map->get_nrplayers() < m_current_player) {
			//  Mmh, my current player is not valid. Maybe the user has loaded a
			//  new map while this tool was active. We set the new player to a
			//  valid one. The sel pointer is the only thing that stays wrong, but
			//  this is not important
			m_current_player = 1;
		}

		Widelands::Coords const old_sp = map->get_starting_pos(m_current_player);

		const Image* player_image = g_gr->images().get(player_pictures[m_current_player - 1]);
		assert(player_image);

		//  check if field is valid
		if (editor_tool_set_starting_pos_callback(map->get_fcoords(center.node), *map)) {
			FieldOverlayManager * overlay_manager = eia.mutable_field_overlay_manager();
			//  remove old overlay if any
			overlay_manager->remove_overlay(old_sp, player_image);

			//  add new overlay
			overlay_manager->register_overlay(
				center.node, player_image, 4, Point(player_image->width() / 2, STARTING_POS_HOTSPOT_Y));

			//  set new player pos
			map->set_starting_pos(m_current_player, center.node);
		}
	}
	return 1;
}

Widelands::PlayerNumber EditorSetStartingPosTool::get_current_player
() const
{
	return m_current_player;
}


void EditorSetStartingPosTool::set_current_player(int32_t const i) {
	m_current_player = i;
	fsel_picsname = player_pictures_small[m_current_player - 1];
	m_current_sel_pic = fsel_picsname;
}
