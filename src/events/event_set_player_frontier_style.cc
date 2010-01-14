/*
 * Copyright (C) 2009-2010 by the Widelands Development Team
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

#include "event_set_player_frontier_style.h"

#include "logic/game.h"
#include "logic/player.h"
#include "logic/game_data_error.h"

#include "profile/profile.h"

namespace Widelands {

Event_Set_Player_Frontier_Style::Event_Set_Player_Frontier_Style
	(Section & s, Editor_Game_Base & egbase)
	: Event_Set_Player_Style(s, egbase)
{
	try {
		char const * const style_name = s.get_safe_string("frontier_style");
		try {
			m_style_index =
				egbase.player(m_player).tribe().frontier_style_index(style_name);
		} catch (Tribe_Descr::Nonexistent) {
			throw game_data_error
				(_("player's tribe does not define frontier style \"%s\""),
				 style_name);
		}
	} catch (_wexception const & e) {
		throw game_data_error(_("(set player frontier style): %s"), e.what());
	}
}


void Event_Set_Player_Frontier_Style::Write
	(Section                    & s,
	 Editor_Game_Base     const & egbase,
	 Map_Map_Object_Saver const & mos)
	const
{
	s.set_string("type", "set_player_frontier_style");
	Event_Set_Player_Style::Write(s, egbase, mos);
	s.set_string
		("frontier_style",
		 egbase.player(m_player).tribe().frontier_style_name(m_style_index));
}


Event::State Event_Set_Player_Frontier_Style::run(Game & game) {
	assert(0 < m_player);
	assert    (m_player <= game.map().get_nrplayers());

	game.player(m_player).m_frontier_style_index = m_style_index;

	return m_state = DONE;
}

}
