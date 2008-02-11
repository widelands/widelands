/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#ifndef EVENT_REVEAL_OBJECTIVE_H
#define EVENT_REVEAL_OBJECTIVE_H

#include "event.h"

namespace Widelands {

class Editor_Game_Base;
class Objective;

struct Event_Reveal_Objective : public Event {
	Event_Reveal_Objective(char const * const Name, State const S)
	: Event(Name, S), m_objective(0)
	{}

	int32_t option_menu(Editor_Interactive &);

	State run(Game *);

	void Read (Section &, Editor_Game_Base       &);
	void Write(Section &, Editor_Game_Base const &) const;

	void set_objective(Objective * objective) {m_objective = objective;}
	Objective * get_objective() {return m_objective;}

private:
	Objective * m_objective;
};

};

#endif
