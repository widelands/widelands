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

#include "event.h"
#include "profile/profile.h"

#include "wexception.h"

namespace Widelands {

Event::Event(Section & s) : Named(s.get_name()) {
	char const * const state_name = s.get_string("state", "init");
	if      (not strcmp(state_name, "init"))
		m_state = Event::INIT;
	else if (not strcmp(state_name, "running"))
		m_state = Event::RUNNING;
	else if (not strcmp(state_name, "done"))
		m_state = Event::DONE;
	else
		throw wexception
			("illegal state \"%s\" (must be one of {init, running, done})",
			 state_name);
}


void Event::set_player(Player_Number) {}


void Event::set_position(Coords) {}


}
