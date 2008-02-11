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

#include "event_set_null_trigger.h"

#include "editor_game_base.h"
#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "interactive_base.h"
#include "map.h"
#include "profile.h"
#include "trigger/trigger_null.h"
#include "wexception.h"

#include "upcast.h"

#define EVENT_VERSION 1

namespace Widelands {

Event_Set_Null_Trigger::Event_Set_Null_Trigger
	(char const * const Name, State const S)
	: Event(Name, S), m_trigger(0), m_setto(true)
{}



Event_Set_Null_Trigger::~Event_Set_Null_Trigger() {
	set_trigger(0);
}


void Event_Set_Null_Trigger::Read(Section & s, Editor_Game_Base & egbase) {
	try {
		const int32_t event_version = s.get_safe_int("version");
		if (event_version == EVENT_VERSION) {
			char const * const trigger_name = s.get_safe_string("trigger");
			if (upcast(Trigger_Null, trig, egbase.map().mtm()[trigger_name])) {
			// Bit Hackish, hopefully the user paid attention
				set_trigger(trig);
				m_setto = s.get_bool("setto", true);
			} else
				throw wexception
					("trigger refers to \"%s\", " "which is not a null trigger",
					 trigger_name);
		} else
			throw wexception("unknown/unhandled version %u", event_version);
	} catch (_wexception const & e) {
		throw wexception("(set null trigger): %s", e.what());
	}
}

void Event_Set_Null_Trigger::Write(Section & s, Editor_Game_Base const &) const {
	assert(m_trigger);
	s.set_string ("type",    "set_null_trigger");
	s.set_int    ("version", EVENT_VERSION);
	s.set_string ("trigger", m_trigger->name());
	if (not m_setto)
		s.set_bool("setto",   false);
}


Event::State Event_Set_Null_Trigger::run(Game* game) {
	assert(m_trigger);

	m_trigger->set_trigger_manually(m_setto);
	m_trigger->check_set_conditions(*game); // forcefully update this trigger

	return m_state = DONE;
}

};
