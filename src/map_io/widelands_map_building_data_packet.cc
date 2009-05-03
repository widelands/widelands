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

#include "widelands_map_building_data_packet.h"

#include "constructionsite.h"
#include "editor_game_base.h"
#include "interactive_base.h"
#include "map.h"
#include "player.h"
#include "economy/request.h"
#include "tribe.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include "upcast.h"

#include <map>

namespace Widelands {

#define LOWEST_SUPPORTED_VERSION           1
#define PRIORITIES_INTRODUCED_IN_VERSION   2
#define CURRENT_PACKET_VERSION             2


void Map_Building_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip) return;
	FileRead fr;
	try {fr.Open(fs, "binary/building");} catch (...) {return;}
	Interactive_Base & ibase = *egbase.get_ibase();
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version >= LOWEST_SUPPORTED_VERSION) {
			Map & map = egbase.map();
			X_Coordinate const width  = map.get_width ();
			Y_Coordinate const height = map.get_height();
			FCoords c;
			for (c.y = 0; c.y < height; ++c.y)
				for (c.x = 0; c.x < width; ++c.x)
					if (fr.Unsigned8()) {
						Player_Number const p                   = fr.Unsigned8 ();
						Serial        const serial              = fr.Unsigned32();
						char  const * const name                = fr.CString   ();
						bool          const is_constructionsite = fr.Unsigned8 ();

						//  No building lives on more than one main place.

						//  Get the tribe and the building index.
						if (Player * const player = egbase.get_safe_player(p)) {
							Tribe_Descr const & tribe = player->tribe();
							Building_Index const index = tribe.building_index(name);
							if (not index)
								throw wexception
									("tribe %s does not define building type \"%s\"",
									 tribe.name().c_str(), name);

							//  Now, create this Building, take extra special care for
							//  constructionsites. All data is read later.
							Building & building =
								ol->register_object<Building>
									(serial,
									 (is_constructionsite ?
									  egbase.warp_constructionsite
									  	(c, p, index, Building_Index::Null())
									  :
									  egbase.warp_building(c, p, index)));

							if (packet_version >= PRIORITIES_INTRODUCED_IN_VERSION)
								read_priorities (building, fr);

							//  Reference the players tribe if in editor.
							ibase.reference_player_tribe(p, &tribe);
						} else
							throw wexception("player %u does not exist", p);
					}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("buildings: %s", e.what());
	}
}


/*
 * Write Function
 */
void Map_Building_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	// now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	// Write buildings and owner, register this with the map_object_saver so that
	// it's data can be saved later.
	Map const &  map    = egbase.map();
	Extent const extent = map.extent();
	iterate_Map_FCoords(map, extent, fc) {
		upcast(Building const, building, fc.field->get_immovable());
		if (building and building->get_position() == fc) {
			//  We only write Buildings.
			//  Buildings can life on only one main position.
			assert(!os->is_object_known(*building));

			fw.Unsigned8(1);
			fw.Unsigned8(building->owner().get_player_number());
			fw.Unsigned32(os->register_object(*building));

			upcast(ConstructionSite const, constructionsite, building);
			fw.CString
				((constructionsite ?
				  constructionsite->building() : building->descr())
				 .name().c_str());
			fw.Unsigned8(static_cast<bool>(constructionsite));

			write_priorities(*building, fw);

		} else
			fw.Unsigned8(0);
	}

	fw.Write(fs, "binary/building");
	// DONE
}

/*
 * Priorities are writen in format:
 0    - ware type (8 bits), for example Request:WARE
 2    - count of priorities for this type (8 bits)
   fish  - ware name (32 bits) //  FIXME ???
   4     - priority assigned to a ware (32 bits)
   water - ware name (32 bits) //  FIXME ???
   1     - ware priority(32 bits)
 0xff - end of ware types
 */
void Map_Building_Data_Packet::write_priorities
	(Building const & building, FileWrite & fw)
{
	fw.Unsigned32(building.get_base_priority());

	std::map<int32_t, std::map<Ware_Index, int32_t> > type_to_priorities;
	std::map<int32_t, std::map<Ware_Index, int32_t> >::iterator it;

	Tribe_Descr const & tribe = building.tribe();
	building.collect_priorities(type_to_priorities);
	for (it = type_to_priorities.begin(); it != type_to_priorities.end(); ++it)
	{
		if (it->second.size() == 0)
			continue;

		// write ware type and priority count
		const int32_t ware_type = it->first;
		fw.Unsigned8(ware_type);
		fw.Unsigned8(it->second.size());

		std::map<Ware_Index, int32_t>::iterator it2;
		for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			std::string name;
			Ware_Index const ware_index = it2->first;
			if (Request::WARE == ware_type)
				name = tribe.get_ware_descr(ware_index)->name();
			else if (Request::WORKER == ware_type)
				name = tribe.get_worker_descr(ware_index)->name();
			else
				throw
					wexception
					("unrecognized ware type %d while writing priorities",
					 ware_type);

			fw.CString(name.c_str());
			fw.Unsigned32(it2->second);
		}
	}

	// write 0xff so the end can be easily identified
	fw.Unsigned8(0xff);
}

void Map_Building_Data_Packet::read_priorities
	(Building & building, FileRead & fr)
{
	building.set_priority(fr.Unsigned32());

	Tribe_Descr const & tribe = building.tribe();
	int32_t ware_type = -1;
	// read ware type
	while (0xff != (ware_type = fr.Unsigned8())) {
		// read count of priorities assigned for this ware type
		const uint8_t count = fr.Unsigned8();
		for (uint8_t i = 0; i < count; ++i) {
			Ware_Index idx;
			if (Request::WARE == ware_type)
				idx = tribe.safe_ware_index(fr.CString());
			else if (Request::WORKER == ware_type)
				idx = tribe.safe_worker_index(fr.CString());
			else
				throw
					wexception
					("unrecognized ware type %d while reading priorities",
					 ware_type);

			building.set_priority(ware_type, idx, fr.Unsigned32());
		}
	}
}

};
