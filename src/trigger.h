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

#ifndef __S__TRIGGER_H
#define __S__TRIGGER_H

#include <string>
#include "error.h"

class Game;
class FileRead;
class FileWrite;

/*
 * A trigger is a switch for events. Each event can register 
 * one or more triggers with himself; when all triggers are set
 * the event runs.
 */
class Trigger {
   public:
      Trigger(void) { m_reference=0; };
      virtual ~Trigger(void) { }

      // virtual functions, implemented by the real triggers
      virtual void check_set_conditions(Game*)=0;
      virtual uint get_id(void)=0; // this function is needed to recreate the correct option window

      // Toggle the triggers state (if it isn't a one timer)
      // and give it a chance to reinitialize
      virtual void reset_trigger(Game*)=0;
      
      // Functions needed by all
      void set_name(const char* name) { m_name=name; }
      void set_name(std::string name) { m_name=name; }
      inline const char* get_name() { return m_name.c_str(); }
      inline bool is_set(void) { return m_is_set; }
      inline bool is_one_time_trigger(void)  { return m_is_one_time_trigger; }
      inline void set_is_one_time_trigger(bool t) { m_is_one_time_trigger=t; }
      inline void incr_reference(void) { ++m_reference; }
      inline void decr_reference(void) { --m_reference; assert(m_reference>=0); }
      inline bool is_unreferenced(void) { return !m_reference; }

      // File functions, to save or load this trigger
      virtual void Write(FileWrite*)=0;
      virtual void Read(FileRead*)=0;
      

   protected:
      // This is only for child classes to toggle the trigger
      inline void set_trigger(bool t) { m_is_set=t; }
      
   private:
      std::string m_name;
      bool        m_is_set;
      bool        m_is_one_time_trigger;    // Can this trigger occur only once?
      int         m_reference;
};

#endif

