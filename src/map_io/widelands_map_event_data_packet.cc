/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "logic/editor_game_base.h"
#include "events/event.h"
#include "events/event_factory.h"
#include "map.h"
#include "profile/profile.h"
#include "widelands_fileread.h"
#include "world.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Map_Event_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip)
		return;

	//  skip, if no events saved
	try {FileRead fr; fr.Open(fs, "event");} catch (...) {return;}

	try {
		Profile prof;
		prof.read("event", 0, fs);

		uint32_t const packet_version =
			prof.get_safe_section("global").get_safe_positive("packet_version");
		if (packet_version <= CURRENT_PACKET_VERSION) {
			Manager<Event> & mem = egbase.map().mem();
			while (Section * const s = prof.get_next_section(0))
				try {
					try {
						mem.register_new(Event_Factory::create(*s, egbase));
					} catch (Manager<Event>::Already_Exists) {
						throw wexception("duplicated");
					}
				} catch (std::exception const & e) {
					throw wexception("%s: %s", s->get_name(), e.what());
				}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
		prof.check_used();
	} catch (_wexception const & e) {
		throw wexception("Events: %s", e.what());
	}
}


void Map_Event_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const)
throw (_wexception)
{
	Profile prof;
	prof.create_section("global").set_int
		("packet_version", CURRENT_PACKET_VERSION);

	Manager<Event> const & mem = egbase.map().mem();
	Manager<Event>::Index const nr_events = mem.size();
	for (Manager<Event>::Index i = 0; i < nr_events; ++i) {
		Event const & e = mem[i];
		Section & s = prof.create_section(e.name().c_str());
		switch (e.m_state) {
		case Event::INIT:                                      break;
		case Event::RUNNING: s.set_string("state", "running"); break;
		case Event::DONE:    s.set_string("state", "done");    break;
		default:
			assert(false);
		}
		e.Write(s, egbase);
	}

	prof.write("event", false, fs);
}

};
