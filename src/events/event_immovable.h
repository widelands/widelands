/*
 * Copyright (C) 2008-2010 by the Widelands Development Team
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

#ifndef EVENT_IMMOVABLE_H
#define EVENT_IMMOVABLE_H

#include "event.h"
#include "logic/immovable.h"

struct Event_Immovable_Option_Menu;

namespace Widelands {

struct Event_Immovable : Event {
	friend struct ::Event_Immovable_Option_Menu;
	Event_Immovable(char const * const Name, State const S)
		: Event(Name, S), m_position(Coords::Null()), m_immovable_type(0)
	{}
	Event_Immovable(Section &, Editor_Game_Base &);

	bool has_option_menu() const {return false;}
	int32_t option_menu(Editor_Interactive &) __attribute__ ((noreturn));

	void Write
		(Section &, Editor_Game_Base const &, Map_Map_Object_Saver const &)
		const;

	State run(Game &);

	void reorigin(Coords const new_origin, Extent const extent) {
		m_position.reorigin(new_origin, extent);
	}

	void set_position(Coords const c) {m_position = c;}

private:
	Coords                  m_position;
	Immovable_Descr const * m_immovable_type;
};

}

#endif
