/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "widelands_map_ware_data_packet.h"

#include "fileread.h"
#include "filewrite.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include "transport.h"
#include "tribe.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "worker.h"

#include <map>


#define CURRENT_PACKET_VERSION 1


Widelands_Map_Ware_Data_Packet::~Widelands_Map_Ware_Data_Packet() {}


void Widelands_Map_Ware_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
   if (skip)
      return;

   FileRead fr;
   try {
      fr.Open(fs, "binary/ware");
	} catch (...) {
      // not there, so skip
      return ;
	}

   // First packet version
   int packet_version=fr.Unsigned16();

   if (packet_version==CURRENT_PACKET_VERSION) {
      // Now the rest data len
      uint nr_files=fr.Unsigned32();

      WareInstance* w;
      for (uint i=0; i<nr_files; i++) {
         w=new WareInstance(0, 0); // data is read somewhere else
         w->init(egbase);
         ol->register_object(egbase, fr.Unsigned32(), w);
		}
      // DONE
      return;
	}
   throw wexception("Unknown version %i in Widelands_Map_Ware_Data_Packet!\n", packet_version);

   assert(0);
}


/*
 * Write Function
 */
void Widelands_Map_Ware_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{

   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // We transverse the map and whenever we find a suitable object, we check if it has wares of some kind
   Map* map=egbase->get_map();
   std::vector<uint> ids;
   for (ushort y=0; y<map->get_height(); y++) {
      for (ushort x=0; x<map->get_width(); x++) {
         Field* f=map->get_field(Coords(x, y));

         // First, check for Flags
         BaseImmovable* imm=f->get_immovable();
         if (imm && imm->get_type()==Map_Object::FLAG) {
            Flag* fl=static_cast<Flag*>(imm);
            for (int i=0; i<fl->m_item_filled; i++) {
               assert(!os->is_object_known(fl->m_items[i].item));
               ids.push_back(os->register_object(fl->m_items[i].item));
				}
			}

         // Now, check for workers
         Bob* b=f->get_first_bob();
         while (b) {
            if (b->get_bob_type()==Bob::WORKER) {
               Worker* w=static_cast<Worker*>(b);
               WareInstance* ware=w->get_carried_item(egbase);
               if (ware) {
                  assert(!os->is_object_known(ware));
                  ids.push_back(os->register_object(ware));
					}
				}
            b=b->get_next_bob();
			}
		}
	}

   // All checked, we only need to save those stuff to disk
   fw.Unsigned32(ids.size());
   for (uint i=0; i<ids.size(); i++)
      fw.Unsigned32(ids[i]);

   fw.Write(fs, "binary/ware");
   // DONE
}
