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

#ifndef EVENT_SET_TIMER_H
#define EVENT_SET_TIMER_H

#include "event.h"
#include "trigger/trigger_time.h"
#include "widelands.h"

namespace Widelands {

struct Editor_Game_Base;

/**
 * This event is able to set a time trigger to trigger after a certain time
 */
struct Event_Set_Timer : public Event, public Referencer<Trigger> {
	Event_Set_Timer(char const * name, State);
	Event_Set_Timer(Section &, Editor_Game_Base &);
	~Event_Set_Timer();

	std::string identifier() const {return "Event (set timer): " + name();}

	bool has_option_menu() const {return false;}
	int32_t option_menu(Editor_Interactive &) __attribute__ ((noreturn));

	State run(Game &);

	void Write(Section &, Editor_Game_Base &) const;

	void set_trigger(Trigger_Time * const new_trigger) {
		if (new_trigger != m_trigger) {
			if   (m_trigger)
				Referencer<Trigger>::unreference  (*m_trigger);
			if (new_trigger)
				Referencer<Trigger>::  reference(*new_trigger);
			m_trigger = new_trigger;
		}
	}
	Trigger_Time * get_trigger() const {return m_trigger;}

private:
	Trigger_Time * m_trigger;
	Duration       m_duration;
};

}

#endif
