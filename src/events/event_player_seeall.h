/*
 * Copyright (C) 2007-2008, 2010 by the Widelands Development Team
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

#ifndef EVENT_PLAYER_SEEALL_H
#define EVENT_PLAYER_SEEALL_H

#include "event.h"
#include "logic/player.h"

namespace Widelands {

/// Abstract base for events switching the see all mode of players.
struct Event_Player_See_All : public Event {
	Event_Player_See_All
		(char const * const Name, State const S)
		: Event(Name, S), m_player(1), m_switch_to_on(true)
	{}
	Event_Player_See_All(Section &, Editor_Game_Base &);

	bool has_option_menu() const {return false;}

	int32_t option_menu(Editor_Interactive &) {return 0;} // dummy to silent gcc

	void Write
		(Section &, Editor_Game_Base const &, Map_Map_Object_Saver const &)
		const;

	State run(Game &);

protected:
	Player_Number m_player;
	bool          m_switch_to_on;
};

}

#endif
