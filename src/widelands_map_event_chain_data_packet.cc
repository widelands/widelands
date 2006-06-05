/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include "editor_game_base.h"
#include "error.h"
#include "event.h"
#include "event_chain.h"
#include "fileread.h"
#include "filewrite.h"
#include "map.h"
#include "map_event_manager.h"
#include "map_eventchain_manager.h"
#include "map_trigger_manager.h"
#include "profile.h"
#include "trigger.h"
#include "trigger_conditional.h"
#include "util.h"
#include "widelands_map_event_chain_data_packet.h"
#include "world.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_EventChain_Data_Packet::~Widelands_Map_EventChain_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_EventChain_Data_Packet::Read(FileSystem* fs, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader*) throw(wexception) {
   if( skip )
      return;

   // Skip, if no triggers saved
   FileRead fr;
   if( !fr.TryOpen( fs, "event_chain" ))
      return;

   Profile prof;
   prof.read( "event_chain", 0, fs );
   Section* s = prof.get_section( "global" );

   /*

   std::string        m_name;
      bool                m_repeating;
      TriggerConditional* m_trigconditional;
      std::vector<Event*> m_events;

      // For running
      uint                m_curevent;
      State               m_state;
 */
   // check packet version
   int packet_version=s->get_int( "packet_version" );
   if(packet_version == CURRENT_PACKET_VERSION) {
      while(( s = prof.get_next_section(0)) ) {
         std::string name = s->get_name();
         EventChain* e = new EventChain();

         // Name
         e->set_name( name.c_str() );

         // Repeating
         e->m_repeating = s->get_safe_bool( "repeating" );

         // TriggerConditional
         std::vector< TriggerConditional_Factory::Token > toklist;
         uint nr_tokens = s->get_safe_int( "nr_conditional_element" );

         char buf[256];
         for( uint i = 0; i < nr_tokens; i++) {
            sprintf(buf, "conditional_element_%02i", i);
            TriggerConditional_Factory::Token tok;
            std::string type = s->get_safe_string( buf );
            tok.data = 0;
            if( type == "trigger" ) {
               tok.token = TriggerConditional_Factory::TRIGGER;
               sprintf(buf, "conditional_element_%02i_data", i);
               std::string trigname = s->get_safe_string( buf );
               Trigger* trig = egbase->get_map()->get_mtm()->get_trigger( trigname.c_str() );
               if( !trig )
                  throw wexception( "Trigger Conditional of Event Chain %s references unknown trigger %s!\n", name.c_str(), trigname.c_str());
               tok.data = trig;
            } else if ( type == ")" ) {
               tok.token = TriggerConditional_Factory::RPAREN;
            } else if ( type == "(" ) {
               tok.token = TriggerConditional_Factory::LPAREN;
            } else if ( type == "XOR" ) {
               tok.token = TriggerConditional_Factory::XOR;
            } else if ( type == "OR" ) {
               tok.token = TriggerConditional_Factory::OR;
            } else if ( type == "AND" ) {
               tok.token = TriggerConditional_Factory::AND;
            } else if ( type == "NOT" ) {
               tok.token = TriggerConditional_Factory::NOT;
            }
            toklist.push_back( tok );
         }
         e->set_trigcond( TriggerConditional_Factory::create_from_infix( e, toklist ));

         // Events
         uint nr_events = s->get_safe_int( "nr_events" );
         for( uint i = 0; i < nr_events; i++) {
            sprintf(buf, "event_%02i", i);
            std::string evname = s->get_safe_string( buf );
            Event* event = egbase->get_map()->get_mem()->get_event( evname.c_str() );
            if( !event )
               throw wexception( "Event Chain %s references unknown event %s!\n", name.c_str(), evname.c_str());
            e->add_event( event );
         }

         // Current event
         e->m_curevent = s->get_safe_int( "current_event" );

         // State
         std::string state = s->get_safe_string("state");
         if( state == "init") e->m_state = EventChain::INIT;
         else if( state == "running") e->m_state = EventChain::RUNNING;
         else if( state == "done") e->m_state = EventChain::DONE;

         egbase->get_map()->get_mecm()->register_new_eventchain( e );
      }
      return;
   }
   throw wexception("Unknown version in Map EventChain Packet: %i\n", packet_version );
}

/*
 * Write Function
 */
void Widelands_Map_EventChain_Data_Packet::Write(FileSystem* fs, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver*) throw(wexception) {
   Profile prof;
   Section* s = prof.create_section( "global" );

   s->set_int("packet_version", CURRENT_PACKET_VERSION );

   // Now write all the event chains
   Map* map = egbase->get_map();
   for(int i=0; i<map->get_mecm()->get_nr_eventchains(); i++) {
      EventChain* e = map->get_mecm()->get_eventchain_by_nr(i);
      s = prof.create_section( e->get_name());

      s->set_bool( "repeating", e->m_repeating );

      std::vector< TriggerConditional_Factory::Token >* toklist =  e->m_trigconditional->get_infix_tokenlist();
      s->set_int("nr_conditional_element", toklist->size());
      char buf[256];
      for( uint i = 0; i < toklist->size(); i++) {
         TriggerConditional_Factory::Token tok = (*toklist)[i];
         sprintf(buf, "conditional_element_%02i", i);
         switch( tok.token ) {
            case TriggerConditional_Factory::OPERATOR:
               assert(0); // In a good world, this can't happen
               break;

            case TriggerConditional_Factory::TRIGGER:
               s->set_string( buf, "trigger");
               sprintf(buf, "conditional_element_%02i_data", i);
               s->set_string( buf, static_cast<Trigger*>(tok.data)->get_name());
               break;

            case TriggerConditional_Factory::RPAREN:
               s->set_string( buf, ")");
               break;

            case TriggerConditional_Factory::LPAREN:
               s->set_string( buf, "(");
               break;

            case TriggerConditional_Factory::XOR:
               s->set_string( buf, "XOR");
               break;

            case TriggerConditional_Factory::OR:
               s->set_string( buf, "OR");
               break;

            case TriggerConditional_Factory::AND:
               s->set_string( buf, "AND");
               break;

            case TriggerConditional_Factory::NOT:
               s->set_string( buf, "NOT");
               break;
         }
      }
      delete toklist;


      // Events
      s->set_int( "nr_events", e->m_events.size());
      for( uint i = 0; i < e->m_events.size(); i++) {
         sprintf(buf, "event_%02i", i);
         s->set_string( buf, e->m_events[i]->get_name());
      }

      // Which is the current event
      s->set_int( "current_event", e->m_curevent );

      // State
      switch( e->m_state ) {
         case EventChain::INIT: s->set_string("state", "init"); break;
         case EventChain::RUNNING: s->set_string("state", "running"); break;
         case EventChain::DONE: s->set_string("state", "done"); break;
      }
   }


   prof.write("event_chain", false, fs );

   // done
}
