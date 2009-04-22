/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "trigger_vision.h"

#include "building.h"
#include "editorinteractive.h"
#include "game.h"
#include "i18n.h"
#include "map.h"
#include "player.h"
#include "profile.h"
#include "wexception.h"

#include "upcast.h"

#define PACKET_VERSION 3

namespace Widelands {

Trigger_Vision::Trigger_Vision(char const * const Name, bool const set)
: Trigger_Player_Area(Name, set)
{}


void Trigger_Vision::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version <= PACKET_VERSION) {
			Trigger_Player_Area::Read(s, egbase);
			require_active_vision = s.get_bool("require_active_vision", false);
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (std::exception const & e) {
		throw wexception("(vision): %s", e.what());
	}
}

void Trigger_Vision::Write(Section & s, Editor_Game_Base const & egbase) const
{
	s.set_string ("type",     "vision");
	s.set_int    ("version",  PACKET_VERSION);
	Trigger_Player_Area::Write(s, egbase);
	if (require_active_vision)
		s.set_bool("require_active_vision", true);
}

void Trigger_Vision::check_set_conditions(Game const & game) {
	Map const & map = game.map();
	if
		(m_player_area.x < 0 or map.get_width () <= m_player_area.x
		 or
		 m_player_area.y < 0 or map.get_height() <= m_player_area.y
		 or
		 m_player_area.player_number <= 0
		 or
		 map.get_nrplayers() < m_player_area.player_number)
		return;

	X_Coordinate const mapwidth = map.get_width();
	Vision const required = 1 + require_active_vision;
	Player const & player = game.player(m_player_area.player_number);
	uint32_t count = 0;
	MapRegion<Area<FCoords> > mr (map, m_player_area);
	do if (required <= player.vision(Map::get_index(mr.location(), mapwidth)))
		if (m_count <= ++count)
			return set_trigger(true);
	while (mr.advance(map));
}

};
