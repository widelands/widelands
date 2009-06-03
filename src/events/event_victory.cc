/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#include "event_victory.h"
#include "wexception.h"
#include "profile/profile.h"
#include "wui/interactive_gamebase.h"
#include "wui/interactive_player.h"

#define EVENT_VERSION 1

namespace Widelands {

Event_Victory::Event_Victory(Section & s, Editor_Game_Base &) : Event(s) {
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version == 1) {
			s.get_safe_bool("advance");
		}
	} catch (std::exception const & e) {
		throw wexception("(victory): %s", e.what());
	}
}

void Event_Victory::Write(Section & s, Editor_Game_Base &) const {
	s.set_string("type", "victory");
	s.set_int("version", EVENT_VERSION);
	s.set_bool("advance", victorious);
}

Event::State Event_Victory::run(Game & g) {
	g.get_ipl()->set_playertype(VICTORIOUS);
	return m_state = DONE;
}

};
