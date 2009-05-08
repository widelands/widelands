/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "widelands_map_bob_data_packet.h"

#include "logic/player.h"
#include "tribe.h"

#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Bob_Data_Packet::ReadBob
	(FileRead              &       fr,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol,
	 Coords                  const coords)
{
	char const * const owner = fr.CString();
	char const * const name  = fr.CString();
	uint8_t subtype = fr.Unsigned8();

	Serial const serial = fr.Unsigned32();

	try {
		if (subtype != Bob::CRITTER && subtype != Bob::WORKER)
			throw wexception("unknown bob type %u", subtype);

		if (not strcmp(owner, "world")) {
			if (subtype != Bob::CRITTER)
				throw wexception("world bob is not a critter!");

			Map   const & map   = egbase.map();
			World const & world = map.world();
			int32_t const idx = world.get_bob(name);
			if (idx == -1)
				throw wexception
					("world %s does not define bob type \"%s\"",
					 world.get_name(), name);
			Bob::Descr const & descr = *world.get_bob_descr(idx);
			if (not (map[coords].get_caps() & descr.movecaps()))
				log
					("WARNING: Found a %s at (%i, %i), but it can not move "
					 "there. Put it somewhere else instead. (Node has movecaps %u, "
					 "bob type has movecaps %u.)\n",
					 descr.name().c_str(), coords.x, coords.y,
					 map[coords].get_caps() & (MOVECAPS_WALK | MOVECAPS_SWIM),
					 descr.movecaps());
			ol->register_object<Bob>(serial, descr.create(egbase, 0, coords));
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
							ol->register_object<Bob>
								(serial,
								 tribe->get_worker_descr(idx)->create_object());
						bob.set_position(egbase, coords);
						bob.init(egbase);
					} else
						throw wexception
							("tribe %s does not define bob type \"%s\"", owner, name);
				} else if (subtype == Bob::CRITTER) {
					int32_t const idx = tribe->get_bob(name);
					if (idx != -1)
						ol->register_object<Bob>
							(serial, egbase.create_bob(coords, idx, tribe));
					else
						throw wexception
							("tribe %s does not define defines bob type \"%s\"",
							 owner, name);
				}
			} else
				throw wexception("tribe \"%s\" does not exist", owner);
		}
	} catch (_wexception const & e) {
		throw wexception
			("%u (owner = \"%s\", name = \"%s\"): %s",
			 serial, owner, name, e.what());
	}
}

void Map_Bob_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	FileRead fr;
	fr.Open(fs, "binary/bob");

	Map & map = egbase.map();
	map.recalc_whole_map(); //  for movecaps checks in ReadBob

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION)
			for (uint16_t y = 0; y < map.get_height(); ++y) {
				for (uint16_t x = 0; x < map.get_width(); ++x) {
					uint32_t const nr_bobs = fr.Unsigned32();

					assert(!map[Coords(x, y)].get_first_bob());

					for (uint32_t i = 0; i < nr_bobs; ++i)
						ReadBob(fr, egbase, skip, ol, Coords(x, y));
				}
			}
		else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("bobs: %s", e.what());
	}
}


void Map_Bob_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	assert(os);

	// now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	// Now, all bob id and registerd it
	// A Field can have more
	// than one bob, we have to take this into account
	//  uint8_t   numbers of bob for field
	//      bob1
	//      bob2
	//      ...
	//      bobn
	Map & map = egbase.map();
	for (uint16_t y = 0; y < map.get_height(); ++y) {
		for (uint16_t x = 0; x < map.get_width(); ++x) {
			std::vector<Bob *> bobarr;

			map.find_bobs //  FIXME clean up this mess!
				(Area<FCoords>(map.get_fcoords(Coords(x, y)), 0), &bobarr);
			fw.Unsigned32(bobarr.size());

			for (uint32_t i = 0; i < bobarr.size(); ++i) {
				// write serial number

				//  a bob can not be owned by two nodes
				assert(not os->is_object_known(*bobarr[i]));

				Serial const reg = os->register_object(*bobarr[i]);

				// Write its owner
				std::string owner_tribe =
					bobarr[i]->descr().get_owner_tribe() ?
					bobarr[i]->descr().get_owner_tribe()->name() : "world";
				fw.String(owner_tribe);
				// Write it's name
				fw.String(bobarr[i]->name());
				// Write it's subtype
				fw.Unsigned8(bobarr[i]->get_bob_type());
				// And it's file register index
				fw.Unsigned32(reg);
			}
		}
	}

	fw.Write(fs, "binary/bob");
}

};
