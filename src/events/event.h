/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef EVENT_H
#define EVENT_H

#include "named.h"
#include "referenced.h"
#include "widelands.h"
#include "widelands_geometry.h"

#include <map>
#include <string>
#include <cstring>

struct Editor_Interactive;
struct Section;

namespace Widelands {

struct Editor_Game_Base;
struct Game;

/**
 * Event is a in game event of some kind
 */
struct Event : public Named, public Referenced<Event> {
	friend struct Map_Event_Data_Packet;

	enum State {
		INIT,
		RUNNING,
		DONE
	};

	Event(char const * const Name, State const S = INIT)
		: Named(Name), m_state(S)
	{}
	Event(Section &);
	virtual ~Event() {}

	virtual int32_t option_menu(Editor_Interactive &) = 0;

	virtual void Write(Section &, Editor_Game_Base &) const = 0;

	virtual void set_player(Player_Number);
	virtual void set_position(Coords);
	virtual State run(Game &) = 0;

	State state() const {return m_state;}

protected:
	State m_state;
};

};

#endif
