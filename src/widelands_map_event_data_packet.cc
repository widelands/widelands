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

#include "widelands_map_event_data_packet.h"

#include "editor_game_base.h"
#include "events/event.h"
#include "events/event_factory.h"
#include "map.h"
#include "profile.h"
#include "widelands_fileread.h"
#include "widelands_map_data_packet_ids.h"
#include "world.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Map_Event_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip) return;

	//  skip, if no events saved
	FileRead fr;
	try {fr.Open(fs, "event");} catch (...) {return;}

	Profile prof;
	prof.read("event", 0, fs);

	try {
		int32_t const packet_version =
			prof.get_section("global")->get_safe_int("packet_version");
		if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			Manager<Event> & mem = egbase->map().mem();
			while (Section * const s = prof.get_next_section(0)) {
				char const * const name = s->get_name();
				try {
					char const * const state_name = s->get_string("state", "init");
					Event::State state;
					if      (not strcmp(state_name, "init"))
						state = Event::INIT;
					else if (not strcmp(state_name, "running"))
						state = Event::RUNNING;
					else if (not strcmp(state_name, "done"))
						state = Event::DONE;
					else
						throw wexception
							("illegal state \"%s\" (must be one of {init, running, "
							 "done})",
							 state_name);
					Event & event =
						Event_Factory::create
						(s->get_safe_string("type"), name, state);
					try {
						mem.register_new(event);
					} catch (Manager<Event>::Already_Exists) {
						throw wexception("duplicated");
					}
					event.Read(*s, *egbase);
				} catch (std::exception const & e) {
					throw wexception("%s: %s", name, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("Events: %s", e.what());
	}
}


void Map_Event_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Map_Map_Object_Saver * const)
throw (_wexception)
{
	Profile prof;
	prof.create_section("global")->set_int
		("packet_version", CURRENT_PACKET_VERSION);

	Manager<Event> const & mem = egbase->map().mem();
	Manager<Event>::Index const nr_events = mem.size();
	for (Manager<Event>::Index i = 0; i < nr_events; ++i) {
		Event const & e = mem[i];
		Section & s = *prof.create_section(e.name().c_str());
		switch (e.m_state) {
		case Event::INIT:                                      break;
		case Event::RUNNING: s.set_string("state", "running"); break;
		case Event::DONE:    s.set_string("state", "done");    break;
		default:
			assert(false);
		}
		e.Write(s, *egbase);
	}

	prof.write("event", false, fs);
}

};
