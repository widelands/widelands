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

#ifndef EVENT_UNHIDE_AREA_H
#define EVENT_UNHIDE_AREA_H

#include "event_player_area.h"

struct Event_Unhide_Area_Option_Menu;

namespace Widelands {

/// Makes an area seen to a player. duration determines how long time the area
/// will be in full vision for the player. Valid values are:
/// 0:        Only gives a static view.
/// Forever:  Gives a full view forever.
/// others:   Gives a full view during the specified duration.
struct Event_Unhide_Area : public Event_Player_Area {
	friend struct ::Event_Unhide_Area_Option_Menu;
	Event_Unhide_Area(char const * const Name, State const S)
		:
		Event_Player_Area
		(Name, S, Player_Area<>(1, Area<>(Coords(0, 0), 5))),
		duration(1 << 14)
	{}

	int32_t option_menu(Editor_Interactive &);

	State run(Game *);

	void Read (Section &, Editor_Game_Base       &);
	void Write(Section &, Editor_Game_Base const &) const;

private:
	Duration duration;
};

};

#endif
