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

#ifndef __S__TRIGGER_TIME_H
#define __S__TRIGGER_TIME_H

#include "trigger_ids.h"
#include "trigger.h"

/*
 * For documentation see the description in editor or trigger_factory.cc
 * or see trigger.h
 */
class Trigger_Time : public Trigger {
   public:
      Trigger_Time();
      ~Trigger_Time();
      
      // one liner functions
      uint get_id(void) { return TRIGGER_TIME; } 

      void check_set_conditions(Game*);
      void reset_trigger(Game*);
      
      // File Functions
      void Write(FileWrite*);
      void Read(FileRead*);
  
      inline void set_wait_time(int i) { m_wait_time=i; }
      inline void set_last_start_time(int i) { m_last_start_time; }
      inline int get_wait_time(void) { return m_wait_time; }
      
   private:
      ulong m_wait_time; // in seconds
      uint m_last_start_time;
};

#endif

