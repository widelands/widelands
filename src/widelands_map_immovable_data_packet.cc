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

#include "editor_game_base.h"
#include "fileread.h"
#include "filewrite.h"
#include "immovable.h"
#include "map.h"
#include <stdint.h>
#include "tribe.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

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

	FileRead fr;
	fr.Open(fs, "binary/immovable");

	Map        & map        = egbase->map();
	World      & world      = map.world  ();
	const Extent map_extent = map.extent ();

   // First packet version
	const uint16_t packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) {
		for (;;) {
         uint32_t reg=fr.Unsigned32();
         if (reg==0xffffffff) break;
			const char * const owner = fr.CString ();
			const char * const name  = fr.CString ();
			const Coords position    = fr.Coords32(map_extent);

			assert(not ol->is_object_known(reg));

			if (strcmp(owner, "world")) {
				if (not skip) { //  We do not load player immovables in normal maps.
               // It is a tribe immovable
					egbase->manually_load_tribe(owner);
					if (const Tribe_Descr * const tribe = egbase->get_tribe(owner))
					{
						int32_t idx = tribe->get_immovable_index(name);
						if (idx != -1)
							ol->register_object
								(egbase,
								 reg,
								 egbase->create_immovable(position, idx, tribe));
						else
							throw wexception
								("Unknown tribe-immovable %s in map, asked for tribe: "
								 "%s!",
								 name, owner);
					} else
						throw wexception("Unknown tribe %s in map!", owner);
				}
			} else {
            // World immovable
            int32_t idx=world.get_immovable_index(name);
				if (idx==-1)
					throw wexception("Unknown world immovable %s in map!", name);
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
(FileSystem &,
 Editor_Game_Base*,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
	throw wexception("Immovable_Data_Packet is obsolete");
}
