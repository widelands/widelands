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
#include <vector>
#include "error.h"
#include "trigger.h"
#include "event_ids.h"

class Game;
class FileRead;
class FileWrite;
class Editor_Game_Base;
class Map;

/*
 * Event is a in game event of some kind 
 */
class Event {
   public:
      Event(void) { };
      virtual ~Event(void) { };

      // virtual functions, implemented by the real events 
      virtual void run(Game*)=0;
      virtual uint get_id(void)=0; // this function is needed to recreate the correct option window

      // Functions needed by all
      void set_name(const char* name) { m_name=name; }
      void set_name(std::string name) { m_name=name; }
      inline const char* get_name() { return m_name.c_str(); }
      
      void register_trigger(Trigger*, Map*, bool up );
      void unregister_trigger(Trigger*, Map*);
      inline int get_nr_triggers(void) { return m_triggers.size(); }         
      bool trigger_exists(Trigger* trig);
      
      inline bool is_one_time_event(void) { return m_is_one_time_event; }
      inline void set_is_one_time_event(bool t) { m_is_one_time_event=t; } 
     
      bool reacts_when_trigger_is_set(Trigger* t);

      // Check if triggers are set
      bool check_triggers(void);
      
      // File functions, to save or load this event 
      virtual void Write(FileWrite*, Editor_Game_Base*)=0;
      virtual void Read(FileRead*, Editor_Game_Base*, bool)=0;
      


   protected:
      // only for child classes
      void write_triggers(FileWrite*, Editor_Game_Base*);
      void read_triggers(FileRead*, Editor_Game_Base*, bool);
      void reinitialize(Game*);
      
   private:
      struct Trigger_Info {
         Trigger* t;
         bool     up;
      };
         
      std::vector<Trigger_Info> m_triggers;
      std::string m_name;
      bool        m_is_one_time_event;    // Can this trigger occur only once?

      
      
 };

#endif

