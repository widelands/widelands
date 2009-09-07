/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef EVENT_ROAD_H
#define EVENT_ROAD_H

#include "event.h"
#include "logic/path.h"

struct Event_Road_Option_Menu;

namespace Widelands {

struct Event_Road : Event {
	friend struct ::Event_Road_Option_Menu;
	Event_Road(char const * const Name, State const S) :
		Event(Name, S), m_player(1), m_fill(true)
	{}
	Event_Road(Section &, Editor_Game_Base &);

	bool has_option_menu() const {return false;}
	int32_t option_menu(Editor_Interactive &) __attribute__ ((noreturn));

	State run(Game &);

	void Write(Section &, Editor_Game_Base &) const;

private:
	Path          m_path;
	Player_Number m_player;
	bool          m_fill;
};

};

#endif
