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

#include "widelands_map_event_chain_data_packet.h"

#include "editor_game_base.h"
#include "events/event.h"
#include "events/event_chain.h"
#include "fileread.h"
#include "filewrite.h"
#include "map.h"
#include "map_event_manager.h"
#include "map_eventchain_manager.h"
#include "map_trigger_manager.h"
#include "profile.h"
#include "trigger/trigger.h"
#include "trigger/trigger_conditional.h"
#include "world.h"


#define CURRENT_PACKET_VERSION 1

Widelands_Map_EventChain_Data_Packet::~Widelands_Map_EventChain_Data_Packet() {}


void Widelands_Map_EventChain_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   if (skip)
      return;

   // Skip, if no triggers saved
   FileRead fr;
   if (!fr.TryOpen(fs, "event_chain"))
      return;

   Profile prof;
   prof.read("event_chain", 0, fs);
   Section* s = prof.get_section("global");

   /*

   std::string        m_name;
      bool                m_repeating;
      TriggerConditional* m_trigconditional;
      std::vector<Event*> m_events;

      // For running
      uint32_t                m_curevent;
      State               m_state;
 */
   // check packet version
	const int32_t packet_version = s->get_int("packet_version");
	if (packet_version == CURRENT_PACKET_VERSION)
		while ((s = prof.get_next_section(0))) {
         std::string name = s->get_name();
         EventChain* e = new EventChain();

         // Name
         e->set_name(name.c_str());

         // Repeating
         e->m_repeating = s->get_safe_bool("repeating");

         // TriggerConditional
         std::vector< TriggerConditional_Factory::Token > toklist;
         uint32_t nr_tokens = s->get_safe_int("nr_conditional_element");

         char buf[256];
			for (uint32_t i = 0; i < nr_tokens; ++i) {
            sprintf(buf, "conditional_element_%02i", i);
            TriggerConditional_Factory::Token tok;
            std::string type = s->get_safe_string(buf);
            tok.data = 0;
				if (type == "trigger") {
               tok.token = TriggerConditional_Factory::TRIGGER;
               sprintf(buf, "conditional_element_%02i_data", i);
               std::string trigname = s->get_safe_string(buf);
               Trigger * const trig = egbase->get_map()->get_mtm().get_trigger(trigname.c_str());
					if (!trig)
						throw wexception
							("Trigger Conditional of Event Chain %s references "
							 "unknown trigger %s!",
							 name.c_str(), trigname.c_str());
               tok.data = trig;
				} else if (type == ")") {
               tok.token = TriggerConditional_Factory::RPAREN;
				} else if (type == "(") {
               tok.token = TriggerConditional_Factory::LPAREN;
				} else if (type == "XOR") {
               tok.token = TriggerConditional_Factory::XOR;
				} else if (type == "OR") {
               tok.token = TriggerConditional_Factory::OR;
				} else if (type == "AND") {
               tok.token = TriggerConditional_Factory::AND;
				} else if (type == "NOT") {
               tok.token = TriggerConditional_Factory::NOT;
				}
            toklist.push_back(tok);
			}
         e->set_trigcond(TriggerConditional_Factory::create_from_infix(e, toklist));

         // Events
         uint32_t nr_events = s->get_safe_int("nr_events");
			for (uint32_t i = 0; i < nr_events; ++i) {
            sprintf(buf, "event_%02i", i);
            std::string evname = s->get_safe_string(buf);
            Event * const event = egbase->get_map()->get_mem().get_event(evname.c_str());
				if (!event)
					throw wexception
						("Event Chain %s references unknown event %s!",
						 name.c_str(), evname.c_str());
            e->add_event(event);
			}

         // Current event
         e->m_curevent = s->get_safe_int("current_event");

         // State
         std::string state = s->get_safe_string("state");
         if (state == "init") e->m_state = EventChain::INIT;
         else if (state == "running") e->m_state = EventChain::RUNNING;
         else if (state == "done") e->m_state = EventChain::DONE;

         egbase->get_map()->get_mecm().register_new_eventchain(e);
		}
	else
		throw wexception
			("Unknown version in Map EventChain Packet: %i", packet_version);
}

/*
 * Write Function
 */
void Widelands_Map_EventChain_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   Profile prof;
	prof.create_section("global")->set_int
		("packet_version", CURRENT_PACKET_VERSION);

   // Now write all the event chains
	const MapEventChainManager & mecm = egbase->get_map()->get_mecm();
	const MapEventChainManager::Index nr_eventchains =
		mecm.get_nr_eventchains();
	for (MapEventChainManager::Index i = 0; i < nr_eventchains; ++i) {
		const EventChain & e = mecm.get_eventchain_by_nr(i);
		Section & s = *prof.create_section(e.name().c_str());
		s.set_bool("repeating", e.m_repeating);
		std::vector< TriggerConditional_Factory::Token >* toklist =
			e.m_trigconditional->get_infix_tokenlist();
		s.set_int("nr_conditional_element", toklist->size());
      char buf[256];
		for (uint32_t t = 0; t < toklist->size(); ++t) {
         TriggerConditional_Factory::Token tok = (*toklist)[t];
         sprintf(buf, "conditional_element_%02i", t);
			s.set_string(buf, TriggerConditional_Factory::operators[tok.token]);
			if (tok.token == TriggerConditional_Factory::TRIGGER) {
               sprintf(buf, "conditional_element_%02i_data", t);
               s.set_string(buf, static_cast<Trigger*>(tok.data)->get_name());
			}
		}
      delete toklist;


      // Events
		const EventChain::event_vector::size_type size =
			e.m_events.size();
		s.set_int("nr_events", size);
		for (EventChain::event_vector::size_type eventnum = 0; eventnum < size; ++eventnum) {
			sprintf(buf, "event_%02u", eventnum);
			s.set_string(buf, e.m_events[eventnum]->name().c_str());
		}

      // Which is the current event
		s.set_int("current_event", e.m_curevent);

      // State
		switch (e.m_state) {
		case EventChain::INIT:    s.set_string("state", "init");    break;
		case EventChain::RUNNING: s.set_string("state", "running"); break;
		case EventChain::DONE:    s.set_string("state", "done");    break;
		}
	}


   prof.write("event_chain", false, fs);

   // done
}
