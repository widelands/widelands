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

#include <map>
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_immovable_data_packet.h"
#include "error.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Immovable_Data_Packet::~Widelands_Map_Immovable_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Immovable_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase) throw(wexception) {
   Map* map=egbase->get_map();
   World* world=map->get_world();

   // First packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      int nr_immovables=fr->Unsigned16();
      if(nr_immovables!=world->get_nr_immovables()) throw wexception("Number of immovables in map (%i) and in world (%i) do not match", 
            nr_immovables, world->get_nr_immovables());

      // construct ids and map
      std::map<uchar,Immovable_Descr*> smap;
      char* buffer;
      for(int i=0; i<nr_immovables; i++) {
         int id=fr->Unsigned16();
         buffer=fr->CString();
         if(!world->get_immovable_descr(world->get_immovable_index(buffer))) throw wexception("Immovable '%s' exists in map, not in world!", buffer);
         smap[id]=world->get_immovable_descr(world->get_immovable_index(buffer));
      }

      // Now get all the the immovables 
      for(ushort y=0; y<map->get_height(); y++) {
         for(ushort x=0; x<map->get_width(); x++) {
            int id=fr->Unsigned16();
            if(id==0xffff) continue; // nothing here
      //      log("[Map Loader] Setting immovable of (%i,%i) to '%s'\n", x, y, smap[id]->get_name());
            egbase->create_immovable(Coords(x, y), id);
         }
      }
      return;
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Immovable_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_IMMOVABLE);

   // now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // This is a bit more complicated saved so that the order of loading
   // of the immovables at run time doens't matter. 
   // (saved like terrains)
   // Write the number of immovables 
   World* world=egbase->get_map()->get_world();
   int nr_ter=world->get_nr_immovables();
   fw->Unsigned16(nr_ter);
  
   // Write all immovable names and their id's
   std::map<std::string,uchar> smap;
   for(int i=0; i<nr_ter; i++) {
      Immovable_Descr* imm=world->get_immovable_descr(i);
      smap[imm->get_name()]=i;
      fw->Unsigned16(i);
      fw->Data(imm->get_name(), strlen(imm->get_name()));
      fw->Unsigned8('\0');
   }
   
   // Now, all immovables as unsigned shorts in order 
   // while ffff means no immovables
   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         BaseImmovable* immovable=map->get_field(Coords(x,y))->get_immovable();
         // The next one is valid since the editor does not register 
         // player immovables with fields, therefore
         // all immovables are map immovables
         Immovable* imm=static_cast<Immovable*>(immovable); 
         if(immovable) {
            // write id
            fw->Unsigned16(smap[imm->get_name()]);
         } else {
            // write ffff
            fw->Unsigned16(0xffff);
         }
      }
   }

   // DONE
}
