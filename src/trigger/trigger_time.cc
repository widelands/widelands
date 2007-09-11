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

#include "trigger_time.h"

#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "profile.h"
#include "wexception.h"

static const int TRIGGER_VERSION = 1;

/*
 * Init and cleanup
 */
Trigger_Time::Trigger_Time()
: Trigger(_("Time Trigger")), m_wait_time(60), m_last_start_time(0)
 // defaults to one minute
{set_trigger(false);}


void Trigger_Time::Read(Section* s, Editor_Game_Base*) {
   int version= s->get_safe_int("version");

   if (version == TRIGGER_VERSION) {
      m_wait_time = s->get_safe_int("wait_time");
      m_last_start_time = s->get_safe_int("last_start_time");
      return;
	}
   throw wexception("Time Trigger with unknown/unhandled version %i in map!\n", version);
}

void Trigger_Time::Write(Section & s) const {
	s.set_int("version",         TRIGGER_VERSION);
	s.set_int("wait_time",       m_wait_time);
	s.set_int("last_start_time", m_last_start_time);
}

/*
 * check if trigger conditions are done
 */
void Trigger_Time::check_set_conditions(Game* game) {
   if (((game->get_gametime()-m_last_start_time)/1000) < m_wait_time) return;

   // Time has come. Set us
   set_trigger(true);
}

/*
 * Reset this trigger. This is only valid for non one timers
 */
void Trigger_Time::reset_trigger(Game* game) {
   // save new start time
   // NOTE: if it took a while for an event to note us,
   // this time the trigger wasn't counting
   m_last_start_time=game->get_gametime();

   set_trigger(false);
}
