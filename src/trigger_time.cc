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

/* 
 * Init and cleanup
 */
Trigger_Time::Trigger_Time(void) {
   m_last_start_time=0;
   m_wait_time=60; // defaults to one minute
   set_name("Time Trigger");
   set_trigger("false");
   set_is_one_time_trigger("false");
}

Trigger_Time::~Trigger_Time(void) {
}

/*
 * File Read, File Write
 */
void Trigger_Time::Read(FileRead* fr) {
   log("TODO: Trigger_Time File Read!\n");
}

void Trigger_Time::Write(FileWrite* fw) {
   log("TODO: Trigger_Time File Write!\n");
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

