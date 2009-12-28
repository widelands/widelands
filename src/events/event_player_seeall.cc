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

#include "event_player_seeall.h"

#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "profile/profile.h"

#define EVENT_VERSION 1

namespace Widelands {

Event_Player_See_All::Event_Player_See_All(Section & s, Editor_Game_Base &)
	: Event(s)
{
	try {
		uint32_t const event_version = s.get_safe_positive("version");
		if (event_version <= EVENT_VERSION) {
			m_switch_to_on  = s.get_bool("switchon", true);
			m_player_number = s.get_int("player", 1);
			if (m_player_number > MAX_PLAYERS)
				throw game_data_error(_("wrong player number %i"), m_player_number);
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), event_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("(player see all): %s"), e.what());
	}
}

Event::State Event_Player_See_All::run(Game & game) {
	game.player(m_player_number).set_see_all(m_switch_to_on);
	return m_state = DONE;
}

void Event_Player_See_All::Write(Section & s, Editor_Game_Base &) const {
	s.set_int   ("version",  EVENT_VERSION);
	s.get_bool  ("switchon", m_switch_to_on);
	if (m_player_number != 1)
		s.set_int("player",    m_player_number);
}


}
