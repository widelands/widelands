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
#include "widelands_map_bob_data_packet.h"
#include "error.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Bob_Data_Packet::~Widelands_Map_Bob_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Bob_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase) throw(wexception) {
   Map* map=egbase->get_map();
   World* world=map->get_world();

   // First packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      int nr_bobs=fr->Unsigned16();
      if(nr_bobs!=world->get_nr_bobs()) throw wexception("Number of bobs in map (%i) and in world (%i) do not match", 
            nr_bobs, world->get_nr_bobs());

      // construct ids and map
      std::map<uchar,Bob_Descr*> smap;
      char* buffer;
      for(int i=0; i<nr_bobs; i++) {
         int id=fr->Unsigned16();
         buffer=fr->CString();
         if(!world->get_bob_descr(world->get_bob(buffer))) throw wexception("Bob '%s' exists in map, not in world!", buffer);
         smap[id]=world->get_bob_descr(world->get_bob(buffer));
      }

      // Now get all the the bobs 
      for(ushort y=0; y<map->get_height(); y++) {
         for(ushort x=0; x<map->get_width(); x++) {
            int nr_bobs=fr->Unsigned8();

            int i=0;
            for(i=0;i<nr_bobs;i++) {
               int id=fr->Unsigned16();
               egbase->create_bob(Coords(x, y), id);
            }
         }
      }
      return;
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Bob_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_BOB);

   // now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // This is a bit more complicated saved so that the order of loading
   // of the bobs at run time doens't matter. 
   // (saved like terrains)
   // Write the number of bobs 
   World* world=egbase->get_map()->get_world();
   int nr_ter=world->get_nr_bobs();
   fw->Unsigned16(nr_ter);
  
   // Write all bob names and their id's
   std::map<std::string,uchar> smap;
   for(int i=0; i<nr_ter; i++) {
      Bob_Descr* bob=world->get_bob_descr(i);
      smap[bob->get_name()]=i;
      fw->Unsigned16(i);
      fw->Data(bob->get_name(), strlen(bob->get_name()));
      fw->Unsigned8('\0');
   }
   
   // Now, all bobs as unsigned shorts in order 
   // A Field can have more
   // than one bob, we have to take this into account
   //  uchar   numbers of bob for field
   //      bob1 
   //      bob2
   //      ... 
   //      bobn
   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         Bob* bob=map->get_field(Coords(x,y))->get_first_bob();
         if(bob) {
            int nrbobs=1;
            while((bob=bob->get_next_bob())) 
               ++nrbobs;
           
            // write number
            fw->Unsigned8(nrbobs);
   
            // write in back directed order (last goes first)
            Bob** bobs=new Bob*[nrbobs];
            int i;
            bob=map->get_field(Coords(x,y))->get_first_bob();
            for(i=nrbobs-1; i>=0; i--) {
               bobs[i]=bob;
               bob=bob->get_next_bob();
            }
            
            for(i=0;i<nrbobs; i++) {
               // write id
               fw->Unsigned16(smap[bobs[i]->get_name()]);
            }
            delete[] bobs;
         } else {
            // Zero bobs
            fw->Unsigned8(0);
         }
      }
   }

   // DONE
}
