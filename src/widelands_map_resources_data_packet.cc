/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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
#include "fileread.h"
#include "filewrite.h"
#include "editor_game_base.h"
#include "map.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"

#include "log.h"

#define CURRENT_PACKET_VERSION 1

Widelands_Map_Resources_Data_Packet::~Widelands_Map_Resources_Data_Packet() {}


void Widelands_Map_Resources_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base * egbase,
 const bool,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   FileRead fr;

   fr.Open(fs, "binary/resource");

   // read packet version
   int packet_version=fr.Unsigned16();
   Map* map=egbase->get_map();
   World* world=egbase->get_map()->get_world();

	if (packet_version == CURRENT_PACKET_VERSION) {
      int nr_res=fr.Unsigned16();
		if (nr_res > world->get_nr_resources())
			log
				("WARNING: Number of resources in map (%i) is bigger than in world "
				 "(%i)",
				 nr_res, world->get_nr_resources());

      // construct ids and map
      std::map<uchar, int> smap;
      char* buffer;
      for (int i=0; i<nr_res; i++) {
         int id=fr.Unsigned16();
         buffer=fr.CString();
         int res=world->get_resource(buffer);
         if (res==-1) throw wexception("Resource '%s' exists in map, not in world!", buffer);
         smap[id]=res;
		}

      // Now get all the the resources
      for (ushort y=0; y<map->get_height(); y++) {
         for (ushort x=0; x<map->get_width(); x++) {
            int id=fr.Unsigned8();
            int found_amount=fr.Unsigned8();
            int start_amount=0;
            int amount=0;
            amount=found_amount;
            start_amount=fr.Unsigned8();

            int set_id, set_amount, set_start_amount;
            // if amount is zero, theres nothing here
            if (!amount) {
               set_id=0;
               set_amount=0;
               set_start_amount=0;
				} else {
               set_id=smap[id];
               set_amount=amount;
               set_start_amount=start_amount;
				}

            // NoLog("[Map Loader] Setting resource of (%i, %i) to '%s'\n", x, y, smap[id]->get_name());
            if (set_id==-1)
               throw("Unkown resource in map file. It is not in world!\n");
            egbase->get_map()->get_field(Coords(x, y))->set_resources(set_id, set_amount);
            egbase->get_map()->get_field(Coords(x, y))->set_starting_res_amount(set_start_amount);
			}
		}
      return;
	}
   assert(0); // never here
}


/*
 * Ok, when we're called from the editor, the default resources
 * are not set, which is ok.
 * When we are called from a game, the default resources are set
 * which is also ok. But this is one reason why save game != saved map
 * in nearly all cases.
 */
void Widelands_Map_Resources_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{

   FileWrite fw;

   // Now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // This is a bit more complicated saved so that the order of loading
   // of the resources at run time doesn't matter.
   // (saved like terrains)
   // Write the number of resources
   World* world=egbase->get_map()->get_world();
   int nr_res=world->get_nr_resources();
   fw.Unsigned16(nr_res);

   // Write all resources names and their id's
   std::map<std::string, uchar> smap;
   for (int i=0; i<nr_res; i++) {
      Resource_Descr* res=world->get_resource(i);
      smap[res->name().c_str()]=i;
      fw.Unsigned16(i);
      fw.CString(res->name().c_str());
	}

   // Now, all resouces as unsigned chars in order
   //  - resource id
   //  - amount
   Map* map=egbase->get_map();
   for (ushort y=0; y<map->get_height(); y++) {
      for (ushort x=0; x<map->get_width(); x++) {
         int res=map->get_field(Coords(x, y))->get_resources();
         int amount=map->get_field(Coords(x, y))->get_resources_amount();
         int start_amount=map->get_field(Coords(x, y))->get_starting_res_amount();
         if (!amount)
            res=0;
         fw.Unsigned8(res);
         fw.Unsigned8(amount);
         fw.Unsigned8(start_amount);
		}
	}

   fw.Write(fs, "binary/resource");
}
