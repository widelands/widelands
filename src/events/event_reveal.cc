/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "wexception.h"

#include "profile.h"

#define EVENT_VERSION 2

namespace Widelands {

void Event_Reveal::Read(Section & s, Editor_Game_Base &) {
	try {
		int32_t const packet_version = s.get_safe_int("version");
		if (1 <= packet_version and packet_version <= EVENT_VERSION)
			reveal = s.get_safe_string("entry");
		else
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (std::exception const & e) {
		throw wexception("(reveal): %s", e.what());
	}
}


void Event_Reveal::Write (Section & s) const {
	s.set_int   ("version", EVENT_VERSION);
	s.set_string("entry",   reveal);
}

};
