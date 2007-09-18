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

#include "widelands_map_flag_data_packet.h"

#include "editor_game_base.h"
#include "fileread.h"
#include "filewrite.h"
#include "map.h"
#include "player.h"
#include "transport.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include <map>


#define CURRENT_PACKET_VERSION 1


Widelands_Map_Flag_Data_Packet::~Widelands_Map_Flag_Data_Packet() {}


void Widelands_Map_Flag_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw
(_wexception)
{
   if (skip)
      return;

   FileRead fr;
	try {fr.Open(fs, "binary/flag");} catch (...) {return;}

   Map* map=egbase->get_map();

	const uint16_t packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) {
      for (ushort y=0; y<map->get_height(); y++) {
         for (ushort x=0; x<map->get_width(); x++) {
            uchar exists=fr.Unsigned8();
            if (exists) {
               // Ok, now read all the additional data
               uchar owner=fr.Unsigned8();
               uint serial=fr.Unsigned32();

               // No flag lives on more than one place
               assert(!ol->is_object_known(serial));

               // Now, create this Flag. Directly
               // create it, do not call the player class since
               // we recreate the data in another packet. We always
               // create this, no matter what skip is since we have
               // to read the data packets. We delete this object
               // later again, if it isn't wanted
               Player* plr = egbase->get_safe_player(owner);
               assert(plr);
               Flag* flag=Flag::create(egbase, plr, Coords(x, y));

               // and register it with the object loader for further loading
               ol->register_object(egbase, serial, flag);
				}
			}

		}
	} else
		throw wexception
			("Unknown version %u in Widelands_Map_Flag_Data_Packet!",
			 packet_version);
}


/*
 * Write Function
 */
void Widelands_Map_Flag_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
   FileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // Write flags and owner, register this with the map_object_saver so that
   // it's data can be saved later.
   Map* map=egbase->get_map();
   for (ushort y=0; y<map->get_height(); y++) {
      for (ushort x=0; x<map->get_width(); x++) {
         BaseImmovable* immovable=map->get_field(Coords(x, y))->get_immovable();
         // We only write flags
         if (immovable && immovable->get_type()==Map_Object::FLAG) {
            Flag* flag=static_cast<Flag*>(immovable);

            // Flags can't life on multiply positions, therefore
            // this flag shouldn't be registered.
            assert(!os->is_object_known(flag));

            uint serial=os->register_object(flag);

            fw.Unsigned8(1);
            fw.Unsigned8(flag->get_owner()->get_player_number());
            // write id
            fw.Unsigned32(serial);

			} else {
            // No existance, no owner
            fw.Unsigned8(0);
			}
		}
	}

   fw.Write(fs, "binary/flag");
   // DONE
}
