/*
 * Copyright (C) 2002-5 by the Widelands Development Team
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

#ifndef __S__EVENT_CHAIN_H
#define __S__EVENT_CHAIN_H

#include <string>
#include <vector>
#include "cmd_queue.h"
#include "event_referencer.h"
#include "error.h"
#include "queue_cmd_ids.h"
#include "trigger_referencer.h"

class Event;
class Game;
class TriggerConditional;

/*
 * The Chain defined 
 * 
 * A eventchain is a succession of events, which are run 
 * in order as soon as the eventchains trigger conditional is 
 * true.
 */
class EventChain : public EventReferencer, public TriggerReferencer {
   friend class Widelands_Map_EventChain_Data_Packet;

   public:
      enum State {
         INIT,
         RUNNING,
         DONE,
      };
      
   public:
      EventChain( ) { 
        m_repeating = false;
        m_trigconditional = 0;
        m_state = INIT;
        m_curevent = 0;
      } 
      virtual ~EventChain( void ) { }

      const wchar_t* get_name( void ) { return m_name.c_str(); }
      const wchar_t* get_type( void ) { return L"EventChain"; }
      inline void set_name( const wchar_t* name ) { m_name = name; }
      inline bool get_repeating( void ) { return m_repeating; }
      inline TriggerConditional* get_trigcond( void ) { return m_trigconditional; }
      inline void set_trigcond( TriggerConditional* t) { m_trigconditional = t; }
     
      inline State get_state( void ) { return m_state; }
      
      // Below is only good idea in editor
      void set_repeating( bool t ) { m_repeating = t; }
      uint get_nr_events( void ) { return m_events.size(); }
      Event* get_event( uint i ) { assert( i < m_events.size()); return m_events[i]; } 
      void clear_events( void );
      void add_event( Event* ev );
     

      // Run this event queue
      State run( Game* g);

   private:
      std::wstring        m_name;
      bool                m_repeating;
      TriggerConditional* m_trigconditional;
      std::vector<Event*> m_events;

      // For running
      uint                m_curevent;
      State               m_state;

};

class Cmd_CheckEventChain :public BaseCommand {
    private:
	int m_eventchain_id;

    public:
   Cmd_CheckEventChain(void) : BaseCommand(0) { } // For savegame loading
	Cmd_CheckEventChain (int, int);
      
   // Write these commands to a file (for savegames)
   virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
   virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

   virtual int get_id(void) { return QUEUE_CMD_CHECK_EVENTCHAIN; } // Get this command id

	virtual void execute (Game*);
};


#endif
