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

#include "widelands_map_event_data_packet.h"
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "error.h"
#include "event.h"
#include "event_factory.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Event_Data_Packet::~Widelands_Map_Event_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Event_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader*) throw(wexception) {
   // read packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      // Get number of events
      int nr_event=fr->Unsigned16();

      // Read all the events
      Map* map=egbase->get_map();
      int i=0;
      for(i=0; i<nr_event; i++) {
         Event* event = Event_Factory::get_correct_event(fr->Unsigned16());
         assert(event);
         event->Read(fr, egbase, skip);
         if(skip)
            delete event;
         else
            map->register_new_event(event);
      }
      return; // done
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Event_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver*) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_EVENT);

   // Now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // Now number of events
   Map* map=egbase->get_map();
   fw->Unsigned16(map->get_number_of_events());

   // Now write all the events
   int i=0;
   for(i=0; i<map->get_number_of_events(); i++)
      map->get_event(i)->Write(fw, egbase);
   // done
}
