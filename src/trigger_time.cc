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

#include "trigger_time.h"
#include "error.h"
#include "filesystem.h"

static const int TRIGGER_VERSION = 1;

/* 
 * Init and cleanup
 */
Trigger_Time::Trigger_Time(void) {
   m_last_start_time=0;
   m_wait_time=60; // defaults to one minute
   set_name("Time Trigger");
   set_trigger(false);
   set_is_one_time_trigger(true);
}

Trigger_Time::~Trigger_Time(void) {
}

/*
 * File Read, File Write
 */
void Trigger_Time::Read(FileRead* fr) {
   int version=fr->Unsigned16();
   if(version <= TRIGGER_VERSION) {
      set_name(fr->CString());
      set_is_one_time_trigger(fr->Unsigned8());
      m_wait_time=fr->Unsigned32();
      return;
   }
   throw wexception("Time Trigger with unknown/unhandled version %i in map!\n", version);
}

void Trigger_Time::Write(FileWrite* fw) {
   // First of all the id
   fw->Unsigned16(get_id());

   // Now the version
   fw->Unsigned16(TRIGGER_VERSION);

   // Name
   fw->Data(get_name(), strlen(get_name()));
   fw->Unsigned8('\0');
   
   // triggers only once?
   fw->Unsigned8(is_one_time_trigger());

   // Wait time
   fw->Unsigned32(m_wait_time);
  
   // done
}
      
/*
 * check if trigger conditions are done
 */
void Trigger_Time::check_set_conditions(Game*) {

}

/*
 * Reset this trigger. This is only valid for non one timers
 */
void Trigger_Time::reset_trigger(void) {
   assert(!is_one_time_trigger());
}

