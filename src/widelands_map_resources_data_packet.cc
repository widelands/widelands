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

#include "widelands_map_resources_data_packet.h"
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "error.h"


#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Resources_Data_Packet::~Widelands_Map_Resources_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Resources_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase) throw(wexception) {
   // read packet version
   int packet_version=fr->Unsigned16();
   Map* map=egbase->get_map();
   World* world=egbase->get_map()->get_world();

   if(packet_version==CURRENT_PACKET_VERSION) {
      int nr_res=fr->Unsigned16();
      if(nr_res>world->get_nr_resources()) throw wexception("Number of resources in map (%i) is bigger than in world (%i)", 
            nr_res, world->get_nr_resources());

      // construct ids and map
      std::map<uchar,Resource_Descr*> smap;
      char* buffer;
      for(int i=0; i<nr_res; i++) {
         int id=fr->Unsigned16();
         buffer=fr->CString();
         int res=world->get_resource(buffer);
         if(res==-1) throw wexception("Resource '%s' exists in map, not in world!", buffer);
         smap[id]=world->get_resource(res);
      }

      // Now get all the the resources 
      for(ushort y=0; y<map->get_height(); y++) {
         for(ushort x=0; x<map->get_width(); x++) {
            
            int id=fr->Unsigned8();
            int amount=fr->Unsigned8();
            // NoLog("[Map Loader] Setting resource of (%i,%i) to '%s'\n", x, y, smap[id]->get_name());
            egbase->get_map()->get_field(Coords(x,y))->set_resources(id,amount);
         }
      }
      return;
   }
   assert(0); // never here
}


/*
 * Write Function
 *
 * Ok, when we're called from the editor, the default resources
 * are not set, which is ok.
 * When we are called from a game, the default resources are set
 * which is also ok. But this is one reason why save game != saved map
 * in nearly all cases.
 */
void Widelands_Map_Resources_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_RESOURCES);

   // Now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // This is a bit more complicated saved so that the order of loading
   // of the resources at run time doesn't matter. 
   // (saved like terrains)
   // Write the number of resources 
   World* world=egbase->get_map()->get_world();
   int nr_res=world->get_nr_resources();
   fw->Unsigned16(nr_res);

   // Write all resources names and their id's
   std::map<std::string,uchar> smap;
   for(int i=0; i<nr_res; i++) {
      Resource_Descr* res=world->get_resource(i);
      smap[res->get_name()]=i;
      fw->Unsigned16(i);
      fw->Data(res->get_name(), strlen(res->get_name()));
      fw->Unsigned8('\0');
   }

   // Now, all resouces as unsigned chars in order 
   //  - resource id
   //  - amount 
   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         int res=map->get_field(Coords(x,y))->get_resources();
         int amount=map->get_field(Coords(x,y))->get_resources_amount();
         fw->Unsigned8(res);
         fw->Unsigned8(amount);
      }
   }
}
