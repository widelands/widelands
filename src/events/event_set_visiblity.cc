/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#include "event_set_visiblity.h"
#include "campvis.h"
#include "i18n.h"
#include "profile.h"
#include "wexception.h"

/**
 * init
 */
Event_Set_Visiblity::Event_Set_Visiblity() : Event(_("Set Visibilty"))
{
	v_entry="";
	v_case=4;
}


/**
 * cleanup
 */
Event_Set_Visiblity::~Event_Set_Visiblity() {}


/**
 * reinitialize
 */
void Event_Set_Visiblity::reinitialize(Game *) {}


/**
 * file read
 */
void Event_Set_Visiblity::Read(Section* s, Editor_Game_Base* egbase)
{
	int32_t version = s->get_safe_int("version");

	if (version == EVENT_VERSION) {
		v_entry = s->get_safe_string("entry");
		v_case  = s->get_safe_int   ("vcase");
	} else {
		throw wexception("set_visibility event with unknown version %i in map!\n", version);
	}
}


/**
 * file write
 */
void Event_Set_Visiblity::Write (Section & s, const Editor_Game_Base &) const
{
	s.set_int   ("version", EVENT_VERSION);
	s.set_string("entry",   v_entry.c_str());
	s.set_int   ("vcase",   v_case);
}


/**
 * run the event
 */
Event::State Event_Set_Visiblity::run(Game* game)
{
	Campaign_visiblity_save cvs;
	cvs.set_visiblity(v_entry, v_case);

	m_state = DONE;
	return m_state;
}
