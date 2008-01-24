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

#ifndef EVENT_REVEAL_CAMPAIGN_H
#define EVENT_REVEAL_CAMPAIGN_H

#include "event_reveal.h"

namespace Widelands {

struct Event_Reveal_Campaign : public Event_Reveal {
	Event_Reveal_Campaign(char const * Name, State const S)
		: Event_Reveal(Name, S)
	{}

	int32_t option_menu(Editor_Interactive &);

	State run(Game*);

	void Write(Section &) const;
};

};

#endif
