/*
 * Copyright (C) 2002-2004, 2006, 2008-2010 by the Widelands Development Team
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

#include "event_player_allowed_retreat_change.h"

#include "logic/game_data_error.h"
#include "logic/editor_game_base.h"
#include "profile/profile.h"
#include "wui/interactive_base.h"

#define EVENT_VERSION 1

namespace Widelands {

Event_Player_Allowed_Retreat_Change::Event_Player_Allowed_Retreat_Change
	(Section & s, Editor_Game_Base &)
	: Event(s)
{
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version <= EVENT_VERSION) {
			//  nothing to do
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error
			(_("(player allowed retreat change): %s"), e.what());
	}
}

void Event_Player_Allowed_Retreat_Change::Write
	(Section & s, Editor_Game_Base &) const
{
	s.set_int           ("version",                EVENT_VERSION);
	if (m_player_number != 1)
		s.set_int        ("player",                 m_player_number);
}


void Event_Player_Allowed_Retreat_Change::set_player (Player_Number const p) {
	m_player_number = p;
}

}
