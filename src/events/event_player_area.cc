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

#include "event_player_area.h"

#include "editor_game_base.h"
#include "map.h"
#include "profile.h"
#include "wexception.h"

#include "log.h"

#define EVENT_VERSION 2

namespace Widelands {

void Event_Player_Area::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		int32_t const event_version = s.get_safe_int("version");
		if (1 <= event_version and event_version <= EVENT_VERSION) {
			Map const & map = egbase.map();
			Extent const extent = map.extent();
			m_player_area = Player_Area<>
				(s.get_Player_Number("player", map.get_nrplayers(), 1),
				 Area<>
				 (event_version == 1
				  ?
				  Coords(s.get_safe_int("point_x"), s.get_safe_int("point_y"))
				  :
				  s.get_safe_Coords("point", extent),
				  s.get_int("area", 0)));
			if
				(m_player_area.x < 0 or extent.w <= m_player_area.x
				 or
				 m_player_area.y < 0 or extent.h <= m_player_area.y)
				throw wexception
					("illegal coordinates (%i, %i)",
					 m_player_area.x, m_player_area.y);
		} else
			throw wexception("unknown/unhandled version %i", event_version);
	} catch (std::exception const & e) {
		throw wexception("(player area): %s", e.what());
	}
}


void Event_Player_Area::Write(Section & s, Editor_Game_Base &) const {
	s.set_int   ("version", EVENT_VERSION);
	s.set_Coords("point",   m_player_area);
	if (m_player_area.radius)
		s.set_int("area",    m_player_area.radius);
	if (m_player_area.player_number != 1)
		s.set_int("player",  m_player_area.player_number);
}

};
