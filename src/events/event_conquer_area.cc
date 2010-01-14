/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "event_conquer_area.h"

#include "logic/game.h"
#include "logic/map.h"

#include "profile/profile.h"

namespace Widelands {

Event_Conquer_Area::Event_Conquer_Area(Section & s, Editor_Game_Base & egbase)
	: Event_Player_Area(s, egbase)
{}


void Event_Conquer_Area::Write
	(Section                    & s,
	 Editor_Game_Base     const & egbase,
	 Map_Map_Object_Saver const & mos)
	const
{
	s.set_string("type", "conquer_area");
	Event_Player_Area::Write(s, egbase, mos);
}


Event::State Event_Conquer_Area::run(Game & game) {
	assert(m_player_area);
	assert(0 < m_player_area.player_number);
	assert    (m_player_area.player_number <= game.map().get_nrplayers());

	game.conquer_area_no_building
		(Player_Area<Area<FCoords> >
		 	(m_player_area.player_number,
		 	 Area<FCoords>
		 	 	(game.map().get_fcoords(m_player_area), m_player_area.radius)));

	return m_state = DONE;
}

}
