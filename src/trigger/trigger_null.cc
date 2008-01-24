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

#include "trigger_null.h"

#include "filesystem.h"
#include "game.h"
#include "i18n.h"
#include "profile.h"
#include "wexception.h"

#define PACKET_VERSION 1

namespace Widelands {

Trigger_Null::Trigger_Null(char const * const Name, bool const set)
: Trigger(Name, set), m_should_toggle(false), m_value(false)
{}


void Trigger_Null::Read(Section & s, Editor_Game_Base &) {
	try {
		int32_t const packet_version = s.get_int("version", PACKET_VERSION);
		if (packet_version != PACKET_VERSION)
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (std::exception const & e) {
		throw wexception("(null): %s", e.what());
	}
}

void Trigger_Null::Write(Section & s) const {
	s.set_string("type", "null");
}

/*
 * check if trigger conditions are done
 */
void Trigger_Null::check_set_conditions(Game const &) {
	if (m_should_toggle) set_trigger(m_value);

   return;
}

/*
 * Reset this trigger. This is only valid for non one timers
 */
void Trigger_Null::reset_trigger       (Game const &) {
   m_should_toggle = false;
   m_value = false;
   set_trigger(false);
}

};
