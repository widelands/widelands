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

#include "widelands_map_event_data_packet.h"

#include "editor_game_base.h"
#include "events/event.h"
#include "events/event_factory.h"
#include "fileread.h"
#include "map.h"
#include "map_event_manager.h"
#include "profile.h"
#include "widelands_map_data_packet_ids.h"
#include "world.h"

#define CURRENT_PACKET_VERSION 1


Widelands_Map_Event_Data_Packet::~Widelands_Map_Event_Data_Packet() {}


void Widelands_Map_Event_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip) return;

   // Skip, if no events saved
		FileRead fr;
	try {fr.Open(fs, "event");} catch (...) {return;}

   Profile prof;
   prof.read("event", 0, fs);
   Section* s = prof.get_section("global");

   // check packet version
			const int packet_version=s->get_int("packet_version");
	if (packet_version == CURRENT_PACKET_VERSION) {
		while ((s = prof.get_next_section(0))) {
         std::string name = s->get_name();
         std::string type = s->get_safe_string("type");
         std::string state = s->get_safe_string("state");
         Event* e = Event_Factory::get_correct_event(type.c_str());
         e->set_name(name.c_str());
					if      (state == "init")    e->m_state = Event::INIT;
					else if (state == "running") e->m_state = Event::RUNNING;
					else if (state == "done")    e->m_state = Event::DONE;

         e->Read(s, egbase);
         egbase->get_map()->get_mem().register_new_event(e);
		}
	} else
		throw wexception
			("Unknown version in Map Event Packet: %u", packet_version);
}

/*
 * Write Function
 */
void Widelands_Map_Event_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   Profile prof;
	prof.create_section("global")->set_int
		("packet_version", CURRENT_PACKET_VERSION);

   // Now write all the events
	const MapEventManager & mem = egbase->get_map()->get_mem();
	const MapEventManager::Index nr_events = mem.get_nr_events();
	for (MapEventManager::Index i = 0; i < nr_events; ++i) {
		const Event & e = mem.get_event_by_nr(i);
		Section & s = *prof.create_section(e.name().c_str());
		s.set_string("type", e.get_id());
		switch (e.m_state) {
		case Event::INIT:    s.set_string("state", "init");    break;
		case Event::RUNNING: s.set_string("state", "running"); break;
		case Event::DONE:    s.set_string("state", "done");    break;
		}
		e.Write(s, *egbase);
	}

   prof.write("event", false, fs);

   // done
}
