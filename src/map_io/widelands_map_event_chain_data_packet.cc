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

#include "widelands_map_event_chain_data_packet.h"

#include "logic/editor_game_base.h"
#include "game_data_error.h"
#include "events/event.h"
#include "events/event_chain.h"
#include "map.h"
#include "profile/profile.h"
#include "trigger/trigger.h"
#include "trigger/trigger_conditional.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "world.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 3


void Map_EventChain_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip)
		return;

	//  Skip, if no triggers saved.
	FileRead fr;
	if (!fr.TryOpen(fs, "event_chain"))
		return;

	Map                 & map = egbase.map();
	Manager<EventChain> & mcm = map.mcm();
	Manager<Event>      & mem = map.mem();
	Manager<Trigger>    & mtm = map.mtm();
	try {
		Profile prof;
		prof.read("event_chain", 0, fs);
		uint32_t const packet_version =
			prof.get_safe_section("global").get_safe_positive("packet_version");
		if (packet_version <= CURRENT_PACKET_VERSION)
			while (Section * const s = prof.get_next_section(0)) {
				char const * const name = s->get_name();
				EventChain & event_chain = *new EventChain(name);
				try {
					try {
						mcm.register_new(event_chain);
					} catch (Manager<EventChain>::Already_Exists) {
						throw game_data_error("duplicated");
					}

					event_chain.m_repeating = s->get_safe_bool("repeating");

					{ //  TriggerConditional
						TriggerConditional::token_vector toklist;
						char key[] = "conditional_element_00\0data"; //  tailed
						while (char const * const type = s->get_string(key)) {
							if (not strcmp(type, "trigger")) {
								key[22] = '_'; //  Enable the tail of the key string.
								char const * const trigname = s->get_safe_string(key);
								key[22] = '\0'; //  Cut off the tail again.
								if (Trigger * const tr = mtm[trigname])
									toklist.push_back
										(TriggerConditional_Factory::Token
										 	(TriggerConditional_Factory::TRIGGER, tr));
								else
									throw game_data_error
										("trigger \"%s\" does not exist", trigname);
							}
							else {
								uint8_t i = 0;
								while
									(strcmp
									 	(type,
									 	 TriggerConditional_Factory::operators[i]))
									if (i++ == TriggerConditional_Factory::TRIGGER)
										throw game_data_error
											("\"%s=%s\": token type \"%s\" is not "
											 "allowed (must be one of {trigger, ), (, "
											 "XOR, OR, AND, NOT}",
											 key, type, type);
								toklist.push_back
									(static_cast<TriggerConditional_Factory::TokenNames>
									 	(i));
							}

							//  Increment the number in the key string.
							if (key[21] == '9') {
								key[21] = '0';
								if (key[20] == '9') //  We are already at number 99!
									throw game_data_error
										("there are too many conditional elements, only "
										 "99 are allowed");
								++key[20];
							} else
								++key[21];
						}
						event_chain.set_trigcond
							(TriggerConditional_Factory::create_from_infix
							 	(event_chain, toklist));
					}

					//  Events
					if (packet_version <= 2) {
						char key[] = "event_00";
						while (char const * const evname = s->get_string(key)) {
							if (Event * const event = mem[evname])
								event_chain.add_event(event);
							else
								throw game_data_error
									("\"%s=%s\": event \"%s\" does not exist",
									 key, evname, evname);

							//  Increment the number in the key string.
							if (key[7] == '9') {
								key[7] = '0';
								if (key[6] == '9') //  We are already at number 99!
									throw game_data_error
										("there are too many events, only 99 are "
										 "allowed");
								++key[6];
							} else
								++key[7];
						}
					} else
						while
							(Section::Value const * const v =
							 	s->get_next_val("event"))
							if (Event * const event = mem[v->get_string()])
								event_chain.add_event(event);
							else
								throw game_data_error
									("event=%s\": event \"%s\" does not exist",
									 v->get_string(), v->get_string());

					event_chain.m_curevent = s->get_safe_int("current_event");

					{ //  state
						char const * const state = s->get_safe_string("state");
						if      (not strcmp(state, "init"))
							event_chain.m_state = EventChain::INIT;
						else if (not strcmp(state, "running"))
							event_chain.m_state = EventChain::RUNNING;
						else if (not strcmp(state, "done"))
							event_chain.m_state = EventChain::DONE;
						else
							throw game_data_error
								("state is \"%s\" but must be one of {init (default), "
								 "running, done}",
								 state);
					}
				} catch (std::exception const & e) {
					throw game_data_error(_("%s: %s"), name, e.what());
				}
			}
		else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
		prof.check_used();
	} catch (std::exception const & e) {
		throw game_data_error(_("EventChains: %s"), e.what());
	}
}


void Map_EventChain_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const)
throw (_wexception)
{
	Profile prof;
	prof.create_section("global").set_int
		("packet_version", CURRENT_PACKET_VERSION);

	Manager<EventChain> const & mcm = egbase.map().mcm();
	Manager<EventChain>::Index const nr_eventchains = mcm.size();
	for (Manager<EventChain>::Index i = 0; i < nr_eventchains; ++i) {
		EventChain const & e = mcm[i];
		Section & s = prof.create_section(e.name().c_str());
		s.set_bool("repeating", e.m_repeating);

		{ //  TriggerConditional
			TriggerConditional::token_vector toklist;
			e.m_trigconditional->get_infix_tokenlist(toklist);
			assert(toklist.size() < 99); //  because we write 2 decimal digits
			char key[] = "conditional_element_00\0data"; //  tailed string
			container_iterate_const(TriggerConditional::token_vector, toklist, j)
			{
				s.set_string
					(key, TriggerConditional_Factory::operators[j.current->token]);
				if (j.current->token == TriggerConditional_Factory::TRIGGER) {
					key[22] = '_'; //  Enable the "_data" tail of the key string.
					s.set_string(key, j.current->data->name());
					key[22] = '\0'; //  Cut off the tail again.
				}

				//  Increment the number in the key string.
				if (key[21] == '9') {key[21] = '0'; ++key[20];} else ++key[21];
			}
		}



		{ //  Events
			EventChain::event_vector const & events = e.m_events;
			container_iterate_const(EventChain::event_vector, events, j)
				s.set_string_duplicate("event", (*j.current)->name());
		}

		s.set_int("current_event", e.m_curevent);

		switch (e.m_state) {
		case EventChain::INIT:    s.set_string("state", "init");    break;
		case EventChain::RUNNING: s.set_string("state", "running"); break;
		case EventChain::DONE:    s.set_string("state", "done");    break;
		default:
			assert(false);
		}
	}

	prof.write("event_chain", false, fs);
}

}
