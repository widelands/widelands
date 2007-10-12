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

#include "fileread.h"
#include "filewrite.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include "transport.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_road_data_packet.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include <map>

#define CURRENT_PACKET_VERSION 1


Widelands_Map_Road_Data_Packet::~Widelands_Map_Road_Data_Packet() {}


void Widelands_Map_Road_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip) return;

   FileRead fr;
	try {fr.Open(fs, "binary/road");} catch (...) {return;}

   const uint16_t packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) {
      uint32_t ser;
      while ((ser=fr.Unsigned32())!=0xffffffff) {
         // If this is already known, get it
         // Road data is read somewhere else
         assert(!ol->is_object_known(ser));
         Road* road=new Road();
         road->init(egbase);
         ol->register_object(egbase, ser, road);
		}
	} else
		throw wexception
			("Unknown version %u in Widelands_Map_Road_Data_Packet!",
			 packet_version);
}


void Widelands_Map_Road_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{

   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // Write roads, register this with the map_object_saver so that
   // it's data can be saved later.
   Map* map=egbase->get_map();
	for (uint16_t y = 0; y < map->get_height(); ++y) {
		for (uint16_t x = 0; x < map->get_width(); ++x) {
         BaseImmovable* immovable=map->get_field(Coords(x, y))->get_immovable();
         // We only write Roads
         if (immovable && immovable->get_type()==Map_Object::ROAD) {
            Road* road=static_cast<Road*>(immovable);

            // Roads can life on multiple positions
            uint32_t serial=0;
            if (os->is_object_known(road)) continue;

            serial=os->register_object(road);
            // write id
            fw.Unsigned32(serial);
			}
		}
	}
   fw.Unsigned32(0xffffffff);

   fw.Write(fs, "binary/road");
   // DONE
}
