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

#include "event_set_player_style.h"

#include "logic/game_data_error.h"
#include "logic/editor_game_base.h"
#include "profile/profile.h"

#define EVENT_VERSION 1

namespace Widelands {

Event_Set_Player_Style::Event_Set_Player_Style
	(Section & s, Editor_Game_Base & egbase)
	: Event(s)
{
	try {
		uint32_t const event_version = s.get_safe_positive("version");
		if (1 <= event_version and event_version <= EVENT_VERSION) {
			m_player =
				s.get_Player_Number("player", egbase.map().get_nrplayers());
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), event_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("(set player style): %s"), e.what());
	}
}


void Event_Set_Player_Style::Write
	(Section & s, Editor_Game_Base const &, Map_Map_Object_Saver const &) const
{
	s.set_int   ("version", EVENT_VERSION);
	if (m_player != 1)
		s.set_int("player",  m_player);
}

}
