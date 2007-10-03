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

#include "widelands_map_building_data_packet.h"

#include "constructionsite.h"
#include "fileread.h"
#include "filewrite.h"
#include "editor_game_base.h"
#include "interactive_base.h"
#include "map.h"
#include "player.h"
#include <stdint.h>
#include "tribe.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include <map>


#define LOWEST_SUPPORTED_VERSION           1
#define PRIORITIES_INTRODUCED_IN_VERSION   2
#define CURRENT_PACKET_VERSION             2

Widelands_Map_Building_Data_Packet::~Widelands_Map_Building_Data_Packet() {}

/*
 * Read Function
 */
void Widelands_Map_Building_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (not skip) {
		FileRead fr;
		try {
			fr.Open(fs, "binary/building");
		} catch (...) {
			// not there, so skip
			return ;
		}

		const Uint16 packet_version = fr.Unsigned16();
		if (packet_version >= LOWEST_SUPPORTED_VERSION) {
			Map & map = egbase->map();
			const X_Coordinate width  = map.get_width ();
			const Y_Coordinate height = map.get_height();
			Player_Area<Area<FCoords> > a;
			for (a.y = 0; a.y < height; ++a.y) for (a.x = 0; a.x < width; ++a.x) {
				if (fr.Unsigned8()) {
					// Ok, now read all the additional data
					a.player_number = fr.Unsigned8();
					int32_t serial=fr.Unsigned32();
					const char * const name = fr.CString();
					bool is_constructionsite=fr.Unsigned8();

					// No building lives on more than one main place
					assert(!ol->is_object_known(serial));

					// Get the tribe and the building index
					Player * const player = egbase->get_safe_player(a.player_number);
					assert(player); // He must be there FIXME Never use assert to validate input!
					const Tribe_Descr & tribe = player->tribe();
					int32_t index= tribe.get_building_index(name);
					if (index==-1)
						throw wexception("Widelands_Map_Building_Data_Packet::Read(): Should create building %s in tribe %s, but building is unknown!\n",
							name, tribe.name().c_str());

					// Now, create this Building, take extra special care for constructionsites
					Building & building = // all data is read later
						*(is_constructionsite ?
						  egbase->warp_constructionsite(a, a.player_number, index, 0)
						  :
						  egbase->warp_building        (a, a.player_number, index));

					if (packet_version >= PRIORITIES_INTRODUCED_IN_VERSION) {
						read_priorities (building, fr);
					}

					// Reference the players tribe if in editor
					egbase->get_iabase()->reference_player_tribe
						(a.player_number, &tribe);
					// and register it with the object loader for further loading
					ol->register_object(egbase, serial, &building);

					a.radius = building.get_conquers();
					if (a.radius) { //  Add to map of military influence.
						Player::Field * const player_fields = player->m_fields;
						const Field & first_map_field = map[0];
						a.field = map.get_field(a);
						MapRegion<Area<FCoords> > mr(map, a);
						do player_fields[mr.location().field - &first_map_field]
							.military_influence
							+=
							egbase->map().calc_influence
							(mr.location(), Area<>(a, a.radius));
						while (mr.advance(map));
					}
				}
			}
		} else
			throw wexception
				("Unknown version %u in Widelands_Map_Building_Data_Packet!",
				 packet_version);
	}
}


/*
 * Write Function
 */
void Widelands_Map_Building_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	// now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	// Write buildings and owner, register this with the map_object_saver so that
	// it's data can be saved later.
	Map* map=egbase->get_map();
	for (uint16_t y=0; y<map->get_height(); y++) {
		for (uint16_t x=0; x<map->get_width(); x++) {
			BaseImmovable* immovable=map->get_field(Coords(x, y))->get_immovable();
			// We only write Buildings
			if (immovable && immovable->get_type()==Map_Object::BUILDING) {
				Building* building=static_cast<Building*>(immovable);

				if (building->get_position()!=Coords(x, y)) {
					// This is not this buildings main position
					fw.Unsigned8('\0');
					continue;
				}

				// Buildings can life on only one main position
				assert(!os->is_object_known(building));
				uint32_t serial=os->register_object(building);

				fw.Unsigned8(1);
				fw.Unsigned8(building->get_owner()->get_player_number());
				// write id
				fw.Unsigned32(serial);

				const ConstructionSite * const constructionsite =
					dynamic_cast<const ConstructionSite *>(building);
				fw.CString
					((constructionsite ?
					  constructionsite->building() : building->descr())
					 .name().c_str());
				fw.Unsigned8(static_cast<bool>(constructionsite));

				write_priorities(*building, fw);

			} else fw.Unsigned8(0);
		}
	}

	fw.Write(fs, "binary/building");
	// DONE
}

/*
 * Priorities are writen in format:
 0    - ware type (8 bits), for example Request:WARE
 2    - count of priorities for this type (8 bits)
   fish  - ware name (32 bits)
   4     - priority assigned to a ware (32 bits)
   water - ware name (32 bits)
   1     - ware priority(32 bits)
 0xff - end of ware types
 */
void Widelands_Map_Building_Data_Packet::write_priorities
(Building & building, FileWrite & fw)
{
	fw.Unsigned32(building.get_base_priority());

	std::map<int32_t, std::map<int32_t, int32_t> > type_to_priorities;
	std::map<int32_t, std::map<int32_t, int32_t> >::iterator it;

	const Tribe_Descr & tribe = building.get_owner()->tribe();
	building.collect_priorities(type_to_priorities);
	for (it = type_to_priorities.begin();
		 it != type_to_priorities.end(); ++it)
	{
		if (it->second.size() == 0)
			continue;

		// write ware type and priority count
		const int32_t ware_type = it->first;
		fw.Unsigned8(ware_type);
		fw.Unsigned8(it->second.size());

		std::map<int32_t, int32_t>::iterator it2;
		for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			std::string name;
			const int32_t ware_index = it2->first;
			if (Request::WARE == ware_type)
				name = tribe.get_ware_descr(ware_index)->name();
			else if (Request::WORKER == ware_type)
				name = tribe.get_worker_descr(ware_index)->name();
			else
				throw wexception("unrecognized ware type %d while writing priorities", ware_type);

			fw.CString(name.c_str());
			fw.Unsigned32(it2->second);
		}
	}

	// write 0xff so the end can be easily identified
	fw.Unsigned8(0xff);
}

void Widelands_Map_Building_Data_Packet::read_priorities
(Building & building, FileRead & fr)
{
	building.set_priority(fr.Unsigned32());

	const Tribe_Descr & tribe = building.get_owner()->tribe();
	int32_t ware_type = -1;
	// read ware type
	while (0xff != (ware_type = fr.Unsigned8())) {
		// read count of priorities assigned for this ware type
		const uint8_t count = fr.Unsigned8();
		for (uint8_t i = 0; i < count; i++) {
			int32_t idx = -1;
			if (Request::WARE == ware_type)
				idx = tribe.get_safe_ware_index(fr.CString());
			else if (Request::WORKER == ware_type)
				idx = tribe.get_safe_worker_index(fr.CString());
			else
				throw wexception("unrecognized ware type %d while reading priorities", ware_type);

			building.set_priority(ware_type, idx, fr.Unsigned32());
		}
	}
}
