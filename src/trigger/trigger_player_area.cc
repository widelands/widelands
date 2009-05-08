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

#include "trigger_building.h"

#include "logic/building.h"
#include "editor/editorinteractive.h"
#include "logic/game.h"
#include "i18n.h"
#include "map.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "wexception.h"

#include "upcast.h"

namespace Widelands {

Trigger_Player_Area::Trigger_Player_Area
	(char const * const Name, bool const set)
:
Trigger      (Name, set),
m_player_area(0, Area<FCoords>(FCoords(Coords(0, 0)), 0)),
m_count      (1)
{}


void Trigger_Player_Area::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		Map const & map = egbase.map();
		Extent const extent = map.extent();
		m_player_area = Player_Area<Area<FCoords> >
			(s.get_Player_Number("player", map.get_nrplayers(), 1),
			 Area<FCoords>
			 	(map.get_fcoords
			 	 	(s.get_safe_int("version") == 1
			 	 	 ?
			 	 	 (Coords(s.get_safe_int("point_x"), s.get_safe_int("point_y")))
			 	 	 :
			 	 	 s.get_safe_Coords("point", extent)),
			 	 s.get_int("area", 0)));
		if
			(m_player_area.x < 0 or extent.w <= m_player_area.x
			 or
			 m_player_area.y < 0 or extent.h <= m_player_area.y)
			throw wexception
				("illegal coordinates (%i, %i)", m_player_area.x, m_player_area.y);
		if ((m_count = s.get_int("count", 1)) < 1)
			throw wexception("illegal count %u, must be positive", m_count);
	} catch (std::exception const & e) {
		throw wexception("(player prea): %s", e.what());
	}
}

void Trigger_Player_Area::Write(Section & s, Editor_Game_Base const &) const {
	s.set_Coords("point",    m_player_area);
	if (m_player_area.radius)
		s.set_int("area",     m_player_area.radius);
	if (m_player_area.player_number != 1)
		s.set_int("player",   m_player_area.player_number);
	if (m_count                     != 1)
		s.set_int("count",    m_count);
}

};
