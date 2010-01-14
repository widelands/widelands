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

#ifndef EVENT_PLAYER_ALLOWED_RETREAT_CHANGE_H
#define EVENT_PLAYER_ALLOWED_RETREAT_CHANGE_H

#include "event.h"

#include <set>

namespace Widelands {

/// Abstract base for events involving a player and a boolean option
struct Event_Player_Allowed_Retreat_Change : public Event {
	Event_Player_Allowed_Retreat_Change
		(char const * const Name, State const S)
		: Event(Name, S), m_player(1)
	{}
	Event_Player_Allowed_Retreat_Change
		(Section &, Editor_Game_Base &);

	virtual char const * action_name() const = 0; /// What the event type does.

	int32_t option_menu(Editor_Interactive &) __attribute__ ((noreturn));

	void Write
		(Section &, Editor_Game_Base const &, Map_Map_Object_Saver const &)
		const;

	Player_Number player() const {return m_player;}
	void set_player(Player_Number const p) {m_player = p;}

protected:
	Player_Number m_player;
};

}

#endif
