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


#include "editorinteractive.h"
#include "trigger_time.h"
#include "trigger_ids.h"
#include "trigger_factory.h"
#include "error.h"
#include "wexception.h"
#include "trigger_time_option_menu.h"

static const int nr_of_triggers=2;

Trigger_Descr TRIGGER_DESCRIPTIONS[nr_of_triggers] = {
   { TRIGGER_TIME, "Time Trigger", "This Trigger waits a certain time before it is true. It can be configured to constantly restart itself when"
                      "the wait time is over for repeating events" },
   { TRIGGER_OWN_AREA, "Own Area Trigger", "This Trigger gets set when the configured field is owned by the configured player. If it isn't a one timer"
                            "it unsets itself again when the area is no longer owned by the player and resets itselt when it is again" },
};


/*
 * return the correct trigger for this id
 */
Trigger* Trigger_Factory::get_correct_trigger(uint id) {
   switch(id) {
      case TRIGGER_TIME: return new Trigger_Time(); break;
      default: break;
   }
   throw wexception("Trigger_Factory::get_correct_trigger: Unknown trigger id found: %i\n", id);
   // never here
   return 0;
}

/*
 * create the correct option dialog and initialize it with the given
 * trigger. if the given trigger is zero, create a new trigger 
 * and let it be initalised through it.
 * if it fails, return zero/unmodified given trigger, elso return the created/modified trigger
 */
Trigger* Trigger_Factory::make_trigger_with_option_dialog(uint id, Editor_Interactive* m_parent, Trigger* trig) {
   if(!trig) 
      trig=get_correct_trigger(id);

   int retval=-100;
   switch(id) {
      case TRIGGER_TIME: { Trigger_Time_Option_Menu* t=new Trigger_Time_Option_Menu(m_parent, static_cast<Trigger_Time*>(trig)); retval=t->run(); delete t; } break; 
      default: break;
   }
   if(retval==-100) 
      throw wexception("Trigger_Factory::make_trigger_with_option_dialog: Unknown trigger id found: %i\n", id);
   if(retval) 
      return trig;
   delete trig;
   return 0;
}

/*
 * Get the correct trigger descriptions and names from the
 * id header
 */   
Trigger_Descr* Trigger_Factory::get_correct_trigger_descr(uint id) {
   assert(id<Trigger_Factory::get_nr_of_available_triggers());
   
   return &TRIGGER_DESCRIPTIONS[id];
}

/*
 * return the nummer of available triggers
 */
const uint Trigger_Factory::get_nr_of_available_triggers(void) {
   return nr_of_triggers;
}

