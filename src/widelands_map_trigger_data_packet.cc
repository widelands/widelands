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

#include "widelands_map_trigger_data_packet.h"
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "error.h"
#include "trigger.h"
#include "trigger_factory.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Trigger_Data_Packet::~Widelands_Map_Trigger_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Trigger_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase) throw(wexception) {
   // read packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      // Get number of triggers
      int nr_trig=fr->Unsigned16();
      
      // Read all the triggers 
      Map* map=egbase->get_map();
      int i=0;
      for(i=0; i<nr_trig; i++) {
         Trigger* trig = Trigger_Factory::get_correct_trigger(fr->Unsigned16());
         assert(trig);
         trig->Read(fr);
         if(get_scenario_skip()) 
            delete trig; // what a waste, but o, we are not requested
         else 
            map->register_new_trigger(trig);
      }
      return; // done
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Trigger_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_TRIGGER);

   // Now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // Now number of triggers
   Map* map=egbase->get_map(); 
   fw->Unsigned16(map->get_number_of_triggers());
   
   // Now write all the triggers
   int i=0;
   for(i=0; i<map->get_number_of_triggers(); i++)
      map->get_trigger(i)->Write(fw);
   // done
}
