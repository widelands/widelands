/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "event_move_view.h"

#include "wui/interactive_player.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "profile/profile.h"

#define EVENT_VERSION 2

namespace Widelands {

Event_Move_View::Event_Move_View(Section & s, Editor_Game_Base & egbase)
	: Event (s)
{
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version <= EVENT_VERSION) {
			Map const & map = egbase.map();
			Extent const extent = map.extent();
			m_position =
				packet_version == 1 ?
				Coords(s.get_safe_int("point_x"), s.get_safe_int("point_y"))
				:
				s.get_safe_Coords("point", extent);
			if
				(m_position.x < 0 or extent.w <= m_position.x
				 or
				 m_position.y < 0 or extent.h <= m_position.y)
				throw game_data_error
					(_("illegal coordinates (%i, %i)"), m_position.x, m_position.y);
			m_player = s.get_Player_Number("player", map.get_nrplayers());
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("(move view): %s"), e.what());
	}
}

void Event_Move_View::Write
	(Section & s, Editor_Game_Base const &, Map_Map_Object_Saver const &) const
{
	s.set_string ("type",    "move_view");
	s.set_int    ("version", EVENT_VERSION);
	s.set_Coords ("point",   m_position);
	if (m_player != 1)
		s.set_int ("player",  m_player);
}


Event::State Event_Move_View::run(Game & game) {
	assert(m_position);

	Interactive_Player & ipl = *game.get_ipl();
	if (ipl.player_number() == m_player)
		ipl.move_view_to(m_position);

	return m_state = DONE;
}

}
