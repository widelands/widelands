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

#include "event_player_seeall.h"

#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "profile/profile.h"

#define EVENT_VERSION 1

namespace Widelands {

Event_Player_See_All::Event_Player_See_All
	(Section & s, Editor_Game_Base & egbase)
	: Event(s)
{
	try {
		uint32_t const event_version = s.get_safe_positive("version");
		if (event_version <= EVENT_VERSION) {
			m_switch_to_on  = s.get_bool("switchon", true);
			m_player        =
				s.get_Player_Number("player", egbase.map().get_nrplayers());
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), event_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("(player see all): %s"), e.what());
	}
}


void Event_Player_See_All::Write
	(Section & s, Editor_Game_Base const &, Map_Map_Object_Saver const &) const
{
	s.set_string("type",     "seeall");
	s.set_int   ("version",  EVENT_VERSION);
	s.get_bool  ("switchon", m_switch_to_on);
	if (m_player != 1)
		s.set_int("player",   m_player);
}


Event::State Event_Player_See_All::run(Game & game) {
	game.player(m_player).set_see_all(m_switch_to_on);
	return m_state = DONE;
}

}
