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

#ifndef __S__EVENT_H
#define __S__EVENT_H

#include <string>
#include <map>
#include "error.h"
#include "trigger.h"
#include "options.h"

class Game;
class Section;
class Editor_Game_Base;
class EventReferencer;
class Map;

/*
 * Event is a in game event of some kind
 */
class Event {
   friend class Widelands_Map_Event_Data_Packet;

   public:
      enum State {
         INIT,
         RUNNING,
         DONE
      };
      
   public:
      Event(void) { 
         m_state = INIT;
      };
      virtual ~Event(void) { };

      // virtual functions, implemented by the real events
      virtual State run( Game* )       = 0;
      virtual void reinitialize(Game*) = 0;             // can be overwritten to reintialize stuff in the child class
      virtual const char* get_id(void)        = 0; // this function is needed to recreate the correct option window

      // Functions needed by all
      void set_name(const wchar_t* name) { m_name = name; }
      inline const wchar_t* get_name() { return m_name.c_str(); }

      // File functions, to save or load this event
      virtual void Write(Section*, Editor_Game_Base*)=0;
      virtual void Read(Section*, Editor_Game_Base*)=0;

      // Reference this event
      void reference( EventReferencer* ref );
      void unreference( EventReferencer* ref);
      inline const std::map<EventReferencer*,uint>& get_referencers( void ) { return m_referencers; }

      inline State get_state( void ) { return m_state; }

   protected:
      State        m_state;
      
   private:
      std::wstring                m_name;
      std::map<EventReferencer*,uint> m_referencers;
 };

#endif

