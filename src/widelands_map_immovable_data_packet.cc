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
#include "immovable.h"
#include "map.h"
#include "tribe.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_immovable_data_packet.h"
#include "error.h"

/* VERSION 1: initial release
   VERSION 2: 
     - registering through Map_Object_Loader/Saver
     - handling for tribe immovables (ignored on skip)
*/

#define CURRENT_PACKET_VERSION 2

/*
 * Destructor
 */
Widelands_Map_Immovable_Data_Packet::~Widelands_Map_Immovable_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Immovable_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader* mol) throw(wexception) {
   Map* map=egbase->get_map();
   World* world=map->get_world();

   // First packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      while(1) {
         uint reg=fr->Unsigned32();
         if(reg==0xffffffff) break;
         std::string owner=fr->CString();
         std::string name=fr->CString();
         int x=fr->Unsigned16();
         int y=fr->Unsigned16();

         assert(!mol->is_object_known(reg));

         if(owner!="world") {
            if(!skip) { // We do not load play immovables in normal maps
               // It is a tribe immovable
               egbase->manually_load_tribe(owner.c_str());
               Tribe_Descr* tribe=egbase->get_tribe(owner.c_str());
               if(!tribe) 
                  throw wexception("Unknown tribe %s in map!\n", owner.c_str());
               int idx=tribe->get_immovable_index(name.c_str());
               if(idx==-1) 
                  throw wexception("Unknown tribe-immovable %s in map, asked for tribe: %s!\n", name.c_str(), owner.c_str());
               Immovable* imm=egbase->create_immovable(Coords(x, y), idx, tribe); 
               mol->register_object(egbase, reg, imm);
            }
         } else {
            // World immovable
            int idx=world->get_immovable_index(name.c_str());
            if(idx==-1) 
               throw wexception("Unknown world immovable %s in map!\n", name.c_str());
            Immovable* imm=egbase->create_immovable(Coords(x, y), idx, 0); 
            if(!skip)
               mol->register_object(egbase, reg, imm);
         }
      }
      return ;
   } else if(packet_version==1) {
      Read_version1(fr,egbase,skip,mol);
      return;
   }
   assert(0); // never here
}

/*
 * Write Function
 */
void Widelands_Map_Immovable_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver* mos) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_IMMOVABLE);

   // now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         BaseImmovable* immovable=map->get_field(Coords(x,y))->get_immovable();
        
         // We do not write player immovables 
         if(immovable && immovable->get_type()==Map_Object::IMMOVABLE) {
            Immovable* imm=static_cast<Immovable*>(immovable);
            Tribe_Descr* tribe=imm->get_owner_tribe();

            assert(!mos->is_object_known(imm));
            uint reg=mos->register_object(imm);
            
            fw->Unsigned32(reg); 
            if(!tribe) 
               fw->CString("world");
            else 
               fw->CString(tribe->get_name());

            fw->CString(imm->get_name().c_str());
            fw->Unsigned16(x);
            fw->Unsigned16(y);
         }
      }
   }

   fw->Unsigned32(0xffffffff);
   // DONE
}


void Widelands_Map_Immovable_Data_Packet::Read_version1(FileRead *fr, Editor_Game_Base* egbase, bool, Widelands_Map_Map_Object_Loader*) {
   Map* map=egbase->get_map();
   World* world=map->get_world();

   int nr_immovables=fr->Unsigned16();
   if(nr_immovables>world->get_nr_immovables()) log("WARNING: Number of immovables in map (%i) is bigger than in world (%i)",
         nr_immovables, world->get_nr_immovables()); // Not necessarily a problem if none of the unknown are placed

   // construct ids and map
   std::map<uchar,int> smap;
   char* buffer;
   for(int i=0; i<nr_immovables; i++) {
      int id=fr->Unsigned16();
      buffer=fr->CString();
      if(!world->get_immovable_descr(world->get_immovable_index(buffer))) {
         log("WARNING: Immovable '%s' exists in map, not in world! skipped!", buffer);
         smap[id]=-1;
      } else {
         smap[id]=world->get_immovable_index(buffer);
      }
   }

   // Now get all the the immovables
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         int id=fr->Unsigned16();
         if(id==0xffff) continue; // nothing here
         //      log("[Map Loader] Setting immovable of (%i,%i) to '%s'\n", x, y, smap[id]->get_name());
         if(smap[id]==-1) throw wexception("Unknown world immovable in map!\n");
         egbase->create_immovable(Coords(x, y), smap[id], 0); 
      }
   }
   return;
}

