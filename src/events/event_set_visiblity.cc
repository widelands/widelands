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


/*
 * Campaign VISIBILITY
 */


/**
 * init
 */
Event_Set_Campaign_Visiblity::Event_Set_Campaign_Visiblity() : Event(_("Set Campaign Visibilty"))
{
	entry="";
	visible=false;
}


/**
 * cleanup
 */
Event_Set_Campaign_Visiblity::~Event_Set_Campaign_Visiblity() {}


/**
 * reinitialize
 */
void Event_Set_Campaign_Visiblity::reinitialize(Game *) {}


/**
 * file read
 */
void Event_Set_Campaign_Visiblity::Read(Section* s, Editor_Game_Base *) {
	const int32_t packet_version = s->get_safe_int("version");
	if (packet_version == EVENT_VERSION) {
		entry = s->get_safe_string("entry");
		visible = s->get_safe_bool("visible");
	} else
		throw wexception
			("set_campaign_visibility event with unknown version %i in map!",
			 packet_version);
}


/**
 * file write
 */
void Event_Set_Campaign_Visiblity::Write (Section & s, const Editor_Game_Base &) const
{
	s.set_int   ("version", EVENT_VERSION);
	s.set_string("entry",   entry.c_str());
	s.set_bool  ("visible", visible);
}


/**
 * run the event
 */
Event::State Event_Set_Campaign_Visiblity::run(Game *) {
	Campaign_visiblity_save cvs;
	cvs.set_campaign_visiblity(entry, visible);

	m_state = DONE;
	return m_state;
}


/*
 * MAP VISIBILITY
 */


/**
 * init
 */
Event_Set_Map_Visiblity::Event_Set_Map_Visiblity() : Event(_("Set Map Visibilty"))
{
	entry="";
	visible=false;
}


/**
 * cleanup
 */
Event_Set_Map_Visiblity::~Event_Set_Map_Visiblity() {}


/**
 * reinitialize
 */
void Event_Set_Map_Visiblity::reinitialize(Game *) {}


/**
 * file read
 */
void Event_Set_Map_Visiblity::Read(Section* s, Editor_Game_Base *) {
	const int32_t packet_version = s->get_safe_int("version");
	if (packet_version == EVENT_VERSION) {
		entry = s->get_safe_string("entry");
		visible = s->get_safe_bool("visible");
	} else
		throw wexception
			("set_map_visibility event with unknown version %i in map!",
			 packet_version);
}


/**
 * file write
 */
void Event_Set_Map_Visiblity::Write (Section & s, const Editor_Game_Base &) const
{
	s.set_int   ("version", EVENT_VERSION);
	s.set_string("entry",   entry.c_str());
	s.set_bool  ("visible", visible);
}


/**
 * run the event
 */
Event::State Event_Set_Map_Visiblity::run(Game *) {
	Campaign_visiblity_save cvs;
	cvs.set_map_visiblity(entry, visible);

	m_state = DONE;
	return m_state;
}
