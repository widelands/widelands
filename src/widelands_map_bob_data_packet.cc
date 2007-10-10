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

#include "widelands_map_bob_data_packet.h"

#include "fileread.h"
#include "filewrite.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include <stdint.h>
#include "tribe.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "world.h"

#include <map>


// VERSION 1:
//   - workers are also handled here, registering through Map_Object_Loader/Saver
#define CURRENT_PACKET_VERSION 1


Widelands_Map_Bob_Data_Packet::~Widelands_Map_Bob_Data_Packet() {}


void Widelands_Map_Bob_Data_Packet::ReadBob
		(FileRead& fr,
		 Editor_Game_Base* egbase,
		 bool skip,
		 Widelands_Map_Map_Object_Loader * ol,
		 Coords coords)
{
	std::string owner = fr.CString();
	std::string name = fr.CString();
	uint8_t subtype = fr.Unsigned8();

	uint32_t reg = fr.Unsigned32();
	assert(not ol->is_object_known(reg));

	Bob* bob = 0;
	if (subtype != Bob::CRITTER && subtype != Bob::WORKER)
		throw wexception
				("Unknown bob type %i in Widelands_Map_Bob_Data_Packet!\n",
				 subtype);

	if (owner == "world") {
		if (subtype != Bob::CRITTER)
			throw wexception("world bob is not a critter!\n");

		int32_t idx = egbase->get_map()->get_world()->get_bob(name.c_str());
		if (idx == -1)
			throw wexception
					("Map defines Bob %s, but world doesn't deliver!\n",
					 name.c_str());

		bob = egbase->create_bob(coords, idx);
	} else {
		if (skip)
			return; // We do no load player bobs when no scenario

		// Make sure that the correct tribe is known and loaded
		egbase->manually_load_tribe(owner.c_str());

		Tribe_Descr* tribe = egbase->get_tribe(owner.c_str());
		if (!tribe)
			throw wexception
					("Map asks for Tribe %s, but world doesn't deliver!\n",
					 owner.c_str());

		if (subtype == Bob::WORKER) {
			int32_t idx = tribe->get_worker_index(name.c_str());
			if (idx == -1)
				throw wexception
						("Map defines Bob %s, but tribe %s doesn't deliver!\n",
						 name.c_str(), owner.c_str());

			Worker_Descr* descr = tribe->get_worker_descr(idx);

			bob = descr->create_object();
			bob->set_position(egbase, coords);
			bob->init(egbase);
		} else if (subtype == Bob::CRITTER) {
			int32_t idx = tribe->get_bob(name.c_str());
			if (idx == -1)
				throw wexception
						("Map defines Bob %s, but tribe %s doesn't deliver!\n",
						 name.c_str(), owner.c_str());

			bob = egbase->create_bob(coords, idx, tribe);
		}
	}

	assert(bob);

	// Register the bob for further loading
	ol->register_object(egbase, reg, bob);
}

void Widelands_Map_Bob_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	FileRead fr;
	fr.Open(fs, "binary/bob");

	Map* map = egbase->get_map();

	// First packet version
	Uint16 packet_version = fr.Unsigned16();

	if (packet_version == CURRENT_PACKET_VERSION) {
		// Now get all the the bobs
		for (uint16_t y = 0; y < map->get_height(); y++) {
			for (uint16_t x = 0; x < map->get_width(); x++) {
				uint32_t nr_bobs = fr.Unsigned32();

				assert(!egbase->get_map()->get_field(Coords(x, y))->get_first_bob());

				for (uint32_t i = 0; i < nr_bobs; i++) {
					ReadBob(fr, egbase, skip, ol, Coords(x, y));
				}
			}
		}
	} else
		throw wexception
				("Widelands_Map_Bob_Data_Packet: Unknown packet version %u\n",
				 packet_version);
}


/*
 * Write Function
 */
void Widelands_Map_Bob_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
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
	Map* map=egbase->get_map();
	for (uint16_t y=0; y<map->get_height(); y++) {
		for (uint16_t x=0; x<map->get_width(); x++) {
			std::vector<Bob*> bobarr;

			map->find_bobs(Area<FCoords>(map->get_fcoords(Coords(x, y)), 0), &bobarr); //  FIXME clean up this mess!
			fw.Unsigned32(bobarr.size());

			for (uint32_t i=0;i<bobarr.size(); i++) {
				// write serial number
				assert(not os->is_object_known(bobarr[i])); // a bob can't be owned by two fields
				const uint32_t reg = os->register_object(bobarr[i]);

				// Write its owner
				std::string owner_tribe = bobarr[i]->descr().get_owner_tribe() ? bobarr[i]->descr().get_owner_tribe()->name() : "world";
				fw.CString(owner_tribe.c_str());
				// Write it's name
				fw.CString(bobarr[i]->name().c_str());
				// Write it's subtype
				fw.Unsigned8(bobarr[i]->get_bob_type());
				// And it's file register index
				fw.Unsigned32(reg);
			}
		}
	}

	fw.Write(fs, "binary/bob");
}
