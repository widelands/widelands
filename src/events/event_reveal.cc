/*
 * Copyright (C) 2008, 2010 by the Widelands Development Team
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

#include "event_reveal.h"
#include "logic/game_data_error.h"

#include "wexception.h"

#include "profile/profile.h"

#include "i18n.h"

#define EVENT_VERSION 2

namespace Widelands {

Event_Reveal::Event_Reveal(Section & s, Editor_Game_Base &) : Event(s) {
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version <= EVENT_VERSION)
			reveal = s.get_safe_string("entry");
		else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error("(reveal): %s", e.what());
	}
}


void Event_Reveal::Write
	(Section & s, Editor_Game_Base const &, Map_Map_Object_Saver const &) const
{
	s.set_int   ("version", EVENT_VERSION);
	s.set_string("entry",   reveal);
}

}
