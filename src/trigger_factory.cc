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
#include "error.h"
#include "i18n.h"
#include "trigger_time.h"
#include "trigger_null.h"
#include "trigger_building.h"
#include "trigger_factory.h"
#include "trigger_time_option_menu.h"
#include "trigger_null_option_menu.h"
#include "trigger_building_option_menu.h"
#include "wexception.h"

static const int nr_of_triggers=3;
// Translations of names and/or description of triggers does not work with _("xxxxx").c_str() - it ends in queer asci-symbols
// So I disabled it - Nasenbaer
// ToDo make it translateable again
Trigger_Descr TRIGGER_DESCRIPTIONS[nr_of_triggers] = {
	{ "time", "Time Trigger", "This Trigger waits a certain time before it is true. It can be configured to constantly restart itself when the wait time is over for repeating events" },
	{ "null", "Null Trigger", "This Trigger never changes its state by itself. It is useful to pass it to some event which changes triggers" },
	{ "building", "Building Trigger", "This trigger gets set when a number of a building type of one player is available in an area." },
/*   { TRIGGER_OWN_AREA, "Own Area Trigger"), "This Trigger gets set when the configured field is owned by the configured player. If it isn't a one timer it unsets itself again when the area is no longer owned by the player and resets itselt when it is again") },*/
};


/*
 * return the correct trigger for this id
 */
Trigger* Trigger_Factory::get_correct_trigger(const char* id) {
   std::string str = id;

   if( str == "time" ) return new Trigger_Time();
   else if( str == "null" ) return new Trigger_Null();
   else if( str == "building" ) return new Trigger_Building();
   else
      throw wexception("Trigger_Factory::get_correct_trigger: Unknown trigger id found: %s\n", id);

   // never here
   return 0;
}

/*
 * create the correct option dialog and initialize it with the given
 * trigger. if the given trigger is zero, create a new trigger
 * and let it be initalised through it.
 * if it fails, return zero/unmodified given trigger, elso return the created/modified trigger
 */
Trigger* Trigger_Factory::make_trigger_with_option_dialog(const char* id, Editor_Interactive* m_parent, Trigger* gtrig) {
   Trigger* trig=gtrig;
   if(!trig)
      trig=get_correct_trigger(id);

	int retval;
   std::string str = id;
	if        (str == "time") {
		Trigger_Time_Option_Menu t
			(m_parent, static_cast<Trigger_Time * const>(trig));
		retval = t.run();
	} else if (str == "null") {
		Trigger_Null_Option_Menu t(m_parent, static_cast<Trigger_Null*>(trig));
		retval = t.run();
	} else if (str == "building") {
		Trigger_Building_Option_Menu t
			(m_parent, static_cast<Trigger_Building*>(trig));
		retval = t.run();
	} else throw wexception
		("Trigger_Factory::make_trigger_with_option_dialog: Unknown trigger id "
		 "found: %s\n",
		 id);
   if(retval)
      return trig;
   if(!gtrig) {
      delete trig;
      return 0;
   } else return gtrig;
   // never here
}

/*
 * Get the correct trigger descriptions and names from the
 * id header
 */
Trigger_Descr* Trigger_Factory::get_correct_trigger_descr( const char* id ) {
   std::string str = id;
   for( uint i = 0; i < Trigger_Factory::get_nr_of_available_triggers(); i++)
      if( TRIGGER_DESCRIPTIONS[i].id == str )
         return &TRIGGER_DESCRIPTIONS[i];

   assert(0); // never here
   return 0;
}

/*
 * Get the trigger descriptions
 */
Trigger_Descr* Trigger_Factory::get_trigger_descr( uint id ) {
   assert( id < Trigger_Factory::get_nr_of_available_triggers());

   return &TRIGGER_DESCRIPTIONS[id];
}


/*
 * return the nummer of available triggers
 */
const uint Trigger_Factory::get_nr_of_available_triggers(void) {
   return nr_of_triggers;
}
