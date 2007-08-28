/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
#include "error.h"
#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "interactive_base.h"
#include "map.h"
#include "map_trigger_manager.h"
#include "profile.h"
#include "trigger/trigger_null.h"
#include "wexception.h"

static const int EVENT_VERSION = 1;

/*
 * Init and cleanup
 */
Event_Set_Null_Trigger::Event_Set_Null_Trigger() {
   set_name(_("Set Null Trigger").c_str());
   m_trigger = 0;
   set_setto(true);
}



Event_Set_Null_Trigger::~Event_Set_Null_Trigger() {
   set_trigger(0);
}

/*
 * reinitialize
 */
void Event_Set_Null_Trigger::reinitialize(Game *) {}

/*
 * File Read, File Write
 */
void Event_Set_Null_Trigger::Read(Section* s, Editor_Game_Base* egbase) {
	const int event_version = s->get_safe_int("version");
	if (event_version == EVENT_VERSION) {
		const std::string triggername = s->get_safe_string("trigger");
		if
			(Trigger_Null * const trig = static_cast<Trigger_Null *>
			 (egbase->map().get_mtm().get_trigger(triggername.c_str())))
		{
			// Bit Hackish, hopefully the user paid attention
			set_trigger(trig);
			set_setto(s->get_bool("setto"));
		} else
			throw wexception
				("Set Null Trigger event with unknown trigger %s in map!",
				 triggername.c_str());
	} else
		throw wexception
			("Set Null Trigger Event with unknown/unhandled version %u in map!",
			 event_version);
}

void Event_Set_Null_Trigger::Write(Section & s, const Editor_Game_Base &) const
{
   assert(m_trigger);
	s.set_int   ("version", EVENT_VERSION);
	s.set_string("trigger", m_trigger->get_name());
	s.set_bool  ("setto",   get_setto());
}

/*
 * Run this trigger
 */
Event::State Event_Set_Null_Trigger::run(Game* game) {
   assert(m_trigger);

   m_trigger->set_trigger_manually(get_setto());
   m_trigger->check_set_conditions(game); // forcefully update this trigger

   m_state = DONE;
   return m_state;
}
