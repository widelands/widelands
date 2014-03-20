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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "map_io/widelands_map_bob_data_packet.h"

#include "logic/player.h"
#include "logic/tribe.h"
#include "logic/world/world.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Bob_Data_Packet::ReadBob
	(FileRead              &       fr,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader &       mol,
	 Coords                  const coords)
{
	char const * const owner = fr.CString();
	char const * const name  = fr.CString();
	uint8_t subtype = fr.Unsigned8();

	Serial const serial = fr.Unsigned32();

	try {
		if (subtype != Bob::CRITTER && subtype != Bob::WORKER)
			throw game_data_error("unknown bob type %u", subtype);

		if (not strcmp(owner, "world")) {
			if (subtype != Bob::CRITTER)
				throw game_data_error("world bob is not a critter!");

			const Map   & map   = egbase.map();
			const World & world = egbase.world();
			int32_t const idx = world.get_bob(name);
			if (idx == -1)
				throw game_data_error("world does not define bob type \"%s\"", name);
			const Bob::Descr & descr = *world.get_bob_descr(idx);
			if (not (map[coords].nodecaps() & descr.movecaps()))
				log
					("WARNING: Found a %s at (%i, %i), but it can not move "
					 "there. Put it somewhere else instead. (Node has movecaps %u, "
					 "bob type has movecaps %u.)\n",
					 descr.name().c_str(), coords.x, coords.y,
					 map[coords].nodecaps() & (MOVECAPS_WALK | MOVECAPS_SWIM),
					 descr.movecaps());
			mol.register_object<Bob>(serial, descr.create(egbase, nullptr, coords));
		} else {
			if (skip)
				return; // We do no load player bobs when no scenario

			// Make sure that the correct tribe is known and loaded
			egbase.manually_load_tribe(owner);

			if (Tribe_Descr const * const tribe = egbase.get_tribe(owner)) {
				if        (subtype == Bob::WORKER)  {
					const Ware_Index idx = tribe->worker_index(name);
					if (idx) {
						Bob & bob =
							mol.register_object<Bob>
								(serial,
								 tribe->get_worker_descr(idx)->create_object());
						bob.set_position(egbase, coords);
						bob.init(egbase);
					} else
						throw game_data_error
							("tribe %s does not define bob type \"%s\"", owner, name);
				} else if (subtype == Bob::CRITTER) {
					int32_t const idx = tribe->get_bob(name);
					if (idx != -1)
						mol.register_object<Bob>
							(serial, egbase.create_bob(coords, idx, tribe));
					else
						throw game_data_error
							("tribe %s does not define defines bob type \"%s\"",
							 owner, name);
				}
			} else
				throw game_data_error("tribe \"%s\" does not exist", owner);
		}
	} catch (const _wexception & e) {
		throw game_data_error
			("%u (owner = \"%s\", name = \"%s\"): %s",
			 serial, owner, name, e.what());
	}
}

void Map_Bob_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader &       mol)
{
	FileRead fr;
	fr.Open(fs, "binary/bob");

	Map & map = egbase.map();
	map.recalc_whole_map(egbase.world()); //  for movecaps checks in ReadBob

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION)
			for (uint16_t y = 0; y < map.get_height(); ++y) {
				for (uint16_t x = 0; x < map.get_width(); ++x) {
					uint32_t const nr_bobs = fr.Unsigned32();

					for (uint32_t i = 0; i < nr_bobs; ++i)
						ReadBob(fr, egbase, skip, mol, Coords(x, y));
				}
			}
		else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("bobs: %s", e.what());
	}
}


void Map_Bob_Data_Packet::Write
	(FileSystem & /* fs */, Editor_Game_Base & /* egbase */, Map_Map_Object_Saver & /* mos */)
{
	throw wexception("bob packet is deprecated");
}

}
