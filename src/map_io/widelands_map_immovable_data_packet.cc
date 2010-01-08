/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "logic/editor_game_base.h"
#include "logic/immovable.h"
#include "logic/map.h"
#include "logic/tribe.h"
#include "logic/world.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include <map>

namespace Widelands {

void Map_Immovable_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	assert(ol);

	FileRead fr;
	fr.Open(fs, "binary/immovable");

	Map        & map        = egbase.map();
	World      & world      = map.world  ();
	const Extent map_extent = map.extent ();

	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (1 == packet_version) {
			for (;;) {
				Serial const serial = fr.Unsigned32();
				if (serial == 0xffffffff) {
					if (not fr.EndOfFile())
						throw game_data_error
							("expected end of file after serial 0xffffffff");
					break;
				}
				const char * const owner = fr.CString ();
				const char * const name  = fr.CString ();
				const Coords position    = fr.Coords32(map_extent);

				if (strcmp(owner, "world")) {
					if (not skip) { //  do not load player immovables in normal maps
						//  It is a tribe immovable
						Tribe_Descr const & tribe =
							egbase.manually_load_tribe(owner);
						int32_t idx = tribe.get_immovable_index(name);
						if (idx != -1)
							ol->register_object
								(serial,
								 egbase.create_immovable(position, idx, &tribe));
						else
							throw game_data_error
								("tribe %s does not define immovable type \"%s\"",
								 owner, name);
					}
				} else {
					//  world immovable
					int32_t const idx = world.get_immovable_index(name);
					if (idx != -1) {
						Immovable & immovable =
							egbase.create_immovable(position, idx, 0);
						if (not skip)
							ol->register_object(serial, immovable);
					} else
						throw game_data_error
							("world %s does not define immovable type \"%s\"",
							 world.get_name(), name);
				}
			}
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("immovable data: %s"), e.what());
	}
}


void Map_Immovable_Data_Packet::Write
	(FileSystem &, Editor_Game_Base &, Map_Map_Object_Saver * const)
throw (_wexception)
{
	throw wexception("Immovable_Data_Packet is obsolete");
}

}
