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

#include "widelands_map_immovable_data_packet.h"

#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "editor_game_base.h"
#include "immovable.h"
#include "map.h"
#include "tribe.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "error.h"

#include <map>


/* VERSION 1: initial release
     - registering through Map_Object_Loader/Saver
     - handling for tribe immovables (ignored on skip)
*/

#define CURRENT_PACKET_VERSION 1


Widelands_Map_Immovable_Data_Packet::~Widelands_Map_Immovable_Data_Packet() {}


void Widelands_Map_Immovable_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	assert(ol);

	WidelandsFileRead fr;
   fr.Open(fs, "binary/immovable");

	Map        & map        = egbase->map();
	World      & world      = map.world  ();
	const Extent map_extent = map.extent ();

   // First packet version
	const Uint16 packet_version=fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) {
		for (;;) {
         uint reg=fr.Unsigned32();
         if (reg==0xffffffff) break;
			const char * const owner = fr.CString ();
			const char * const name  = fr.CString ();
			const Coords position    = fr.Coords32(map_extent);

			assert(not ol->is_object_known(reg));

			if (strcmp(owner, "world")) {
				if (not skip) { //  We do not load player immovables in normal maps.
               // It is a tribe immovable
					egbase->manually_load_tribe(owner);
					Tribe_Descr* tribe=egbase->get_tribe(owner);
               if (!tribe)
                  throw wexception("Unknown tribe %s in map!\n", owner);
               int idx=tribe->get_immovable_index(name);
               if (idx==-1)
                  throw wexception("Unknown tribe-immovable %s in map, asked for tribe: %s!\n", name, owner);
					ol->register_object
						(egbase, reg, egbase->create_immovable(position, idx, tribe));
				}
			} else {
            // World immovable
            int idx=world.get_immovable_index(name);
            if (idx==-1)
               throw wexception("Unknown world immovable %s in map!\n", name);
				Immovable & immovable = *egbase->create_immovable(position, idx, 0);
				if (not skip) ol->register_object(egbase, reg, &immovable);
			}
		}
	} else
		throw wexception
			("Unknown version %i in Widelands_Map_Immovable_Data_Packet!",
			 packet_version);
}

/*
 * Write Function
 */
void Widelands_Map_Immovable_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
	WidelandsFileWrite fw;

   // now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

	const Map & map = egbase->map();
	const X_Coordinate mapwidth  = map.get_width ();
	const Y_Coordinate mapheight = map.get_height();
	Map::Index i = 0;
	Coords position;
	for (position.y = 0; position.y < mapheight; ++position.y)
		for (position.x = 0; position.x < mapwidth; ++position.x, ++i) {
         // We do not write player immovables
			if
				(const Immovable * const immovable =
				 dynamic_cast<const Immovable * const>(map[i].get_immovable()))
			{
				assert(not os->is_object_known(immovable));
				fw.Unsigned32(os->register_object(immovable));
				if (const Tribe_Descr * const tribe = immovable->get_owner_tribe())
					fw.CString(tribe->name().c_str());
				else fw.CString("world");

				fw.CString(immovable->name().c_str());
				fw.Coords32(position);
			}
		}

   fw.Unsigned32(0xffffffff);

   fw.Write(fs, "binary/immovable");
   // DONE
}
