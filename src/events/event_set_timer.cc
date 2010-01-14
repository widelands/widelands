/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "event_set_timer.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "i18n.h"
#include "wui/interactive_base.h"
#include "logic/map.h"
#include "profile/profile.h"
#include "trigger/trigger_time.h"
#include "wexception.h"

#include "upcast.h"

#define EVENT_VERSION 1

namespace Widelands {

Event_Set_Timer::Event_Set_Timer
	(char const * const Name, State const S)
	: Event(Name, S), m_trigger(0), m_duration(0)
{}


Event_Set_Timer::Event_Set_Timer(Section & s, Editor_Game_Base & egbase)
	: Event(s), m_trigger(0)
{
	try {
		uint32_t const event_version = s.get_safe_positive("version");
		if (event_version == EVENT_VERSION) {
			char const * const trigger_name = s.get_safe_string("trigger");
			if (upcast(Trigger_Time, trig, egbase.map().mtm()[trigger_name]))
				set_trigger(trig);
			else
				throw game_data_error
					(_("trigger \"%s\": not a time trigger"), trigger_name);
			m_duration = s.get_natural("duration", 0);
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), event_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("(set_timer): %s"), e.what());
	}
}


void Event_Set_Timer::Write
	(Section & s, Editor_Game_Base const &, Map_Map_Object_Saver const &) const
{
	assert(m_trigger);
	s.set_string ("type",     "set_timer");
	s.set_int    ("version",  EVENT_VERSION);
	s.set_string ("trigger",  m_trigger->name());
	if (m_duration)
		s.set_int ("duration", m_duration);
}


Event::State Event_Set_Timer::run(Game & game) {
	assert(m_trigger);

	m_trigger->set_time(game.get_gametime() + m_duration);
	m_trigger->check_set_conditions(game); //  forcefully update this trigger

	return m_state = DONE;
}

}
