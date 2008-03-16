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

#include "event_reveal_objective.h"

#include "editor_game_base.h"
#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "interactive_base.h"
#include "map.h"
#include "profile.h"
#include "wexception.h"

#define EVENT_VERSION 2

namespace Widelands {

void Event_Reveal_Objective::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		int32_t const packet_version = s.get_safe_int("version");
		if (1 <= packet_version and packet_version <= EVENT_VERSION) {
			char const * const objective_name = s.get_safe_string("objective");
			if (Objective * const obj = egbase.map().mom()[objective_name])
				set_objective(obj);
			else
				throw wexception("objecive \"%s\" does not exist", objective_name);
		} else
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (std::exception const & e) {
		throw wexception
			("Event (Reveal Objective) %s: %s", name().c_str(), e.what());
	}
}

void Event_Reveal_Objective::Write(Section & s, Editor_Game_Base &) const
{
	assert(m_objective);
	s.set_string("type",      "reveal_objective");
	s.set_int   ("version",   EVENT_VERSION);
	s.set_string("objective", m_objective->name());
}

/*
 * check if trigger conditions are done
 */
Event::State Event_Reveal_Objective::run(Game *) {
	assert(m_objective);

	m_objective->set_is_visible(true);

	return m_state = DONE;
}

};
