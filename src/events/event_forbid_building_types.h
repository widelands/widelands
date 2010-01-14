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

#ifndef EVENT_FORBID_BUILDING_TYPES_H
#define EVENT_FORBID_BUILDING_TYPES_H

#include "event_player_building_types.h"

namespace Widelands {

struct Event_Forbid_Building_Types : public Event_Player_Building_Types {
	Event_Forbid_Building_Types(char const * const Name, State const S) :
		Event_Player_Building_Types(Name, S)
	{}
	Event_Forbid_Building_Types
		(Section           &       s,
		 Editor_Game_Base  &       egbase,
		 Tribe_Descr const * const tribe = 0)
		: Event_Player_Building_Types(s, egbase, tribe)
	{}

	char const * action_name() const;

	void Write
		(Section &, Editor_Game_Base const &, Map_Map_Object_Saver const &)
		const;

	State run(Game &);
};

}

#endif
