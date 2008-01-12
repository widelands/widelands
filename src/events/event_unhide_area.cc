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

#include "event_unhide_area.h"

#include "game.h"
#include "player.h"
#include "profile.h"

namespace Widelands {

Event::State Event_Unhide_Area::run(Game* game) {
	assert(m_player_area);
	assert(0 < m_player_area.player_number);
	assert    (m_player_area.player_number <= game->map().get_nrplayers());

	if (duration)
		game->player(m_player_area.player_number).add_areawatcher(m_player_area)
		.schedule_act(game, duration);
	else game->player(m_player_area.player_number).see_area
		(Player_Area<Area<FCoords> >
		 (m_player_area.player_number,
		  Area<FCoords>
		  (game->map().get_fcoords(m_player_area), m_player_area.radius)),
		 false);

   m_state = DONE;
   return m_state;
}

void Event_Unhide_Area::Read(Section * s, Editor_Game_Base * egbase) {
	Event_Player_Area::Read(s, egbase);
	duration = s->get_int("duration", 0);
}

void Event_Unhide_Area::Write
(Section & s, const Editor_Game_Base & egbase) const
{
	Event_Player_Area::Write(s, egbase);
	s.set_int("duration", duration);
}

};
