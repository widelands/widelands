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
#include "widelands_map_terrain_data_packet.h"
#include "error.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Terrain_Data_Packet::~Widelands_Map_Terrain_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Terrain_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader*) throw(wexception) {
   Map* map=egbase->get_map();
   World* world=map->get_world();

   // first packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      int nr_terrains=fr->Unsigned16();
      if(nr_terrains>world->get_nr_terrains()) throw wexception("Number of terrains in file (%i) is bigger than in world (%i)",
            nr_terrains, world->get_nr_terrains());

      // construct ids and map
      std::map<uchar,Terrain_Descr*> smap;
      char* buffer;
      for(int i=0; i<nr_terrains; i++) {
         int id=fr->Unsigned16();
         buffer=fr->CString();
         if(!world->get_terrain(buffer)) throw wexception("Terrain '%s' exists in map, not in world!", buffer);
         smap[id]=world->get_terrain(buffer);
      }

      // Now get all the terrains
      for(ushort y=0; y<map->get_height(); y++) {
         for(ushort x=0; x<map->get_width(); x++) {
            uchar terd;
            uchar terr;
            Field* f=map->get_field(Coords(x,y));
            terr=fr->Unsigned8();
            terd=fr->Unsigned8();
           // log("[Map Loader] Setting terrain of (%i,%i) to '%s','%s'\n", x, y, smap[terr]->get_name(), smap[terd]->get_name());
            f->set_terrainr(smap[terr]);
            f->set_terraind(smap[terd]);
         }
      }
      return;
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Terrain_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver*) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_TERRAINS);

   // now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // This is a bit more complicated saved so that the order of loading
   // of the terrains at run time doens't matter.
   // This is slow like hell.
   // Write the number of terrains
   World* world=egbase->get_map()->get_world();
   int nr_ter=world->get_nr_terrains();
   fw->Unsigned16(nr_ter);

   // Write all terrain names and their id's
   std::map<std::string,uchar> smap;
   for(int i=0; i<nr_ter; i++) {
      Terrain_Descr* ter=world->get_terrain(i);
      smap[ter->get_name()]=i;
      fw->Unsigned16(i);
      fw->CString(ter->get_name());
   }

   // Now, all terrains as unsigned chars in order
   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         fw->Unsigned8(smap[map->get_field(Coords(x,y))->get_terr()->get_name()]);
         fw->Unsigned8(smap[map->get_field(Coords(x,y))->get_terd()->get_name()]);
      }
   }

   // DONE
}
