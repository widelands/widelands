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

#include "trigger_null.h"
#include "error.h"
#include "filesystem.h"
#include "game.h"

static const int TRIGGER_VERSION = 1;

/*
 * Init and cleanup
 */
Trigger_Null::Trigger_Null(void) {
   set_name("Null Trigger");
   set_trigger(false);
   set_is_one_time_trigger(true);

   m_value=m_should_toggle=false;
}

Trigger_Null::~Trigger_Null(void) {
}

/*
 * File Read, File Write
 */
void Trigger_Null::Read(FileRead* fr, Editor_Game_Base*) {
   int version=fr->Unsigned16();
   if(version <= TRIGGER_VERSION) {
      set_name(fr->CString());
      return;
   }
   throw wexception("Null Trigger with unknown/unhandled version %i in map!\n", version);
}

void Trigger_Null::Write(FileWrite* fw) {
   // First of all the id
   fw->Unsigned16(get_id());

   // Now the version
   fw->Unsigned16(TRIGGER_VERSION);

   // Name
   fw->Data(get_name(), strlen(get_name()));
   fw->Unsigned8('\0');

   // done
}

/*
 * check if trigger conditions are done
 */
void Trigger_Null::check_set_conditions(Game* game) {
   if(m_should_toggle)
      set_trigger(m_value);

   return;
}

/*
 * Reset this trigger. This is only valid for non one timers
 */
void Trigger_Null::reset_trigger(Game* game) {
   // This shouldn't be called, this is a one time trigger always
   assert(0);
}
