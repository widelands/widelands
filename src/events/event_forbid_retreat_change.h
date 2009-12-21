/*
 * Copyright (C) 2002-2004, 2006, 2008-2009 by the Widelands Development Team
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

#ifndef EVENT_FORBID_RETREAT_CHANGE_H
#define EVENT_FORBID_RETREAT_CHANGE_H

#include "event_player_allowed_retreat_change.h"

namespace Widelands {

struct Event_Forbid_Retreat_Change :
	public Event_Player_Allowed_Retreat_Change {
	Event_Forbid_Retreat_Change(char const * const Name, State const S) :
		Event_Player_Allowed_Retreat_Change(Name, S)
	{}
	Event_Forbid_Retreat_Change
		(Section           &       s,
		 Editor_Game_Base  &       egbase)
		: Event_Player_Allowed_Retreat_Change(s, egbase)
	{}

	char const * action_name() const;

	State run(Game &);

	void Write(Section &, Editor_Game_Base &) const;
	};

}

#endif
