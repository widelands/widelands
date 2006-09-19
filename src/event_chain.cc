/*
 * Copyright (C) 2002-2006 by the Widelands Development Team
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

#include "event.h"
#include "event_chain.h"
#include "fileread.h"
#include "filewrite.h"
#include "map.h"
#include "map_event_manager.h"
#include "map_eventchain_manager.h"
#include "map_trigger_manager.h"
#include "game.h"
#include "trigger_conditional.h"
#include "util.h"

/*
 * Run this event chain
 */
EventChain::State EventChain::run( Game* g ) {
   m_state = RUNNING;

   while( m_curevent < m_events.size() ) {
      Event* ev = m_events[m_curevent];
      Event::State retval = ev->run( g );

      if( retval == Event::DONE )
         m_curevent++;
      else break;
   }

   if( m_curevent == m_events.size()) {
      // Last event has been run. This is finished
      if( get_repeating() ) {
         // This eventchain will repeat in due time
         m_curevent = 0;
         m_trigconditional->reset_triggers( g );
         m_state = INIT;
      } else {
         // This eventchain is completly done
         m_state = DONE;
      }
   } else {
      assert( m_events[m_curevent]->get_state() == Event::RUNNING );
   }

   return m_state;
}

/*
 * Clear all events, events are not delted.
 */
void EventChain::clear_events( void ) {
   for( uint i = 0; i < m_events.size(); i++)
      m_events[i]->unreference( this );

   m_events.resize(0);
}

/*
 * Add an event
 */
void EventChain::add_event( Event* ev ) {
   m_events.push_back( ev );
   ev->reference( this );
}

#include "cmd_check_eventchain.h"

/*
 * Check trigger cmd
 */
Cmd_CheckEventChain::Cmd_CheckEventChain (int t, int tid):BaseCommand (t)
{
	m_eventchain_id=tid;
}

void Cmd_CheckEventChain::execute (Game* g)
{
	m_eventchain_id++;

	log("Event Chain: looking if check is needed for eventchain %i\n", m_eventchain_id);

	const Map & map = *g->get_map();
	MapEventChainManager & mecm = map.get_mecm();
	MapEventChainManager::Index nr_eventchains = mecm.get_nr_eventchains();
	if (m_eventchain_id >= nr_eventchains) {
      // either we wrapped around the end of all eventchains
      // if so, restart. if there are no eventchains at all,
      // requeue in about 30 seconds to check if this state has changed
      // (a new trigger could be registered) (this should only happen at the beginning
      // of the game and should not harm at all, and 30seconds means nearly no CPU time
      // for non trigger games)

		if (nr_eventchains) m_eventchain_id = 0;
		else
			return g->enqueue_command
			(new Cmd_CheckEventChain(g->get_gametime() + 30000, -1));
   }

	EventChain & evchain = mecm.get_eventchain_by_nr(m_eventchain_id);

	log("Eventchain %s is going to get checked!\n", evchain.get_name());

	switch (evchain.get_state()) {
      case EventChain::INIT:
      {
         // This is initialized, look if it needs running
         if (evchain.get_trigcond()->eval(g)) {
            // Hooray, we can start the shit off
            evchain.run(g);
         }
      }
      break;

      case EventChain::RUNNING:
      {
         // This chain is currently running. Continue to run it
         evchain.run(g);
      }
      break;

      case EventChain::DONE:
      {
         // This shouldn't happen!
         throw wexception("Cmd_CheckEventChain: Done event chain found. no good no good!\n");
      }
      break;
   }

	if (evchain.get_state() == EventChain::DONE) {
		mecm.delete_eventchain(evchain.get_name());
		nr_eventchains = mecm.get_nr_eventchains();
		map.get_mem().delete_unreferenced_events();
		map.get_mtm().delete_unreferenced_triggers();
   }

	// recheck next in the time that all eventchains get checked at least once ever 10 seconds
	const int delay = nr_eventchains ? 1000 / nr_eventchains : 30000;
   log("Queueing recheck in %i milli-seconds\n", delay);

   g->enqueue_command (new Cmd_CheckEventChain(g->get_gametime() + delay, m_eventchain_id));
}

#define CMD_CHECK_EVENTCHAIN_VERSION 1
void Cmd_CheckEventChain::Read(FileRead* fr, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Loader* mol) {
 int version=fr->Unsigned16();
   if(version==CMD_CHECK_EVENTCHAIN_VERSION) {
      // Read Base Commands
      BaseCommand::BaseCmdRead(fr,egbase,mol);

      // eventchain id
      m_eventchain_id=fr->Unsigned16();
   } else
      throw wexception("Unknown version in Cmd_CheckEventChain::Read: %i", version);
}
void Cmd_CheckEventChain::Write(FileWrite *fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) {
   // First, write version
   fw->Unsigned16(CMD_CHECK_EVENTCHAIN_VERSION);
   // Write base classes
   BaseCommand::BaseCmdWrite(fw, egbase, mos);

   // Now eventchain id
   fw->Unsigned16(m_eventchain_id);
}
