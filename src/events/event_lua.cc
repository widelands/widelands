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

#include "event_lua.h"

#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "profile/profile.h"
#include "scripting/scripting.h"

#define EVENT_VERSION 1


namespace Widelands {
	Event_Lua::Event_Lua(Section & s, Editor_Game_Base &)
		: Event(s)
{
	try {
		uint32_t const event_version = s.get_safe_positive("version");
		if (event_version <= EVENT_VERSION) {
			m_cmd = s.get_safe_string("cmd");
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), event_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("(lua event): %s"), e.what());
	}
}

Event::State Event_Lua::run(Game & game) {
	log("Lua Event ##########\n");
	log("%s\n", m_cmd.c_str());
	game.lua().interpret_string(m_cmd);
	log("##############\n");

	return m_state = DONE;
}

void Event_Lua::Write
	(Section & s, Editor_Game_Base const &, Map_Map_Object_Saver const &) const
{
	s.set_string("type",  "lua");
	s.set_int("version",  EVENT_VERSION);
	s.set_string("cmd", m_cmd);
}


}
