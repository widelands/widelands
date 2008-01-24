/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#ifndef __S__EVENT_PLAYER_AREA_H
#define __S__EVENT_PLAYER_AREA_H

#include "event.h"
#include "player_area.h"

namespace Widelands {

///  Abstract base for events involving a player and an area.
struct Event_Player_Area : public Event {
	Event_Player_Area
		(char const * const Name, State const S,
		 Player_Area<> const player_area =
		 Player_Area<>(1, Area<>(Coords(0, 0), 0)))
		: Event(Name, S), m_player_area(player_area)
	{}

	void Read (Section &, Editor_Game_Base &);
	void Write(Section &) const;

protected:
	Player_Area<> m_player_area;
};

};

#endif
