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

#include "map_io/map_building_packet.h"

#include <map>

#include "base/macros.h"
#include "economy/request.h"
#include "graphic/graphic.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/constructionsite.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribes/tribe_descr.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/interactive_base.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 3;
constexpr uint16_t kLowestSupportedVersion = 1;
constexpr uint16_t kPrioritiesIntroducedInVersion = 2;

// constants to handle special building types
constexpr uint8_t kTypeBuilding = 0;
constexpr uint8_t kTypeConstructionSite = 1;
constexpr uint8_t kTypeDismantleSite = 2;

void MapBuildingPacket::read(FileSystem& fs,
									  EditorGameBase& egbase,
									  bool const skip,
									  MapObjectLoader& mol) {
	if (skip)
		return;
	FileRead fr;
	try {fr.open(fs, "binary/building");} catch (...) {return;}
	InteractiveBase & ibase = *egbase.get_ibase();
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version >= kLowestSupportedVersion && packet_version <= kCurrentPacketVersion) {
			Map & map = egbase.map();
			uint16_t const width  = map.get_width ();
			uint16_t const height = map.get_height();
			FCoords c;
			for (c.y = 0; c.y < height; ++c.y) {
				for (c.x = 0; c.x < width; ++c.x) {
					if (fr.unsigned_8()) {
						PlayerNumber  const p                   = fr.unsigned_8 ();
						Serial        const serial              = fr.unsigned_32();
						char  const * const name                = fr.c_string   ();
						uint8_t const building_type             = fr.unsigned_8 ();

						//  No building lives on more than one main place.

						//  Get the tribe and the building index.
						if (Player * const player = egbase.get_safe_player(p)) {
							const TribeDescr & tribe = player->tribe();
							const BuildingIndex index = tribe.building_index(name);
							if (!tribe.has_building(index)) {
								throw GameDataError
									("tribe %s does not define building type \"%s\"",
									 tribe.name().c_str(), name);
							}

							//  Now, create this Building, take extra special care for
							//  constructionsites. All data is read later.
							Building * building;
							if (building_type == kTypeConstructionSite) {
								building = &egbase.warp_constructionsite(c, p, index, true);
							} else if (building_type == kTypeDismantleSite) {
								Building::FormerBuildings formers = {index};
								building = &egbase.warp_dismantlesite(c, p, true, formers);
							} else {
								building = &egbase.warp_building(c, p, index);
							}

							mol.register_object<Building> (serial, *building);

							if (packet_version >= kPrioritiesIntroducedInVersion) {
								read_priorities (*building, fr);
							}

							//  Reference the players tribe if in editor.
							if (g_gr) // but not on dedicated servers ;)
								ibase.reference_player_tribe(p, &tribe);
						} else
							throw GameDataError("player %u does not exist", p);
					}
				}
			}
		} else {
			throw UnhandledVersionError("MapBuildingPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw GameDataError("buildings: %s", e.what());
	}
}


/*
 * Write Function
 */
void MapBuildingPacket::write
	(FileSystem & fs, EditorGameBase & egbase, MapObjectSaver & mos)
{
	FileWrite fw;

	// now packet version
	fw.unsigned_16(kCurrentPacketVersion);

	// Write buildings and owner, register this with the map_object_saver so that
	// it's data can be saved later.
	const Map &  map    = egbase.map();
	Extent const extent = map.extent();
	iterate_Map_FCoords(map, extent, fc) {
		upcast(Building const, building, fc.field->get_immovable());
		if (building && building->get_position() == fc) {
			//  We only write Buildings.
			//  Buildings can life on only one main position.
			assert(!mos.is_object_known(*building));

			fw.unsigned_8(1);
			fw.unsigned_8(building->owner().player_number());
			fw.unsigned_32(mos.register_object(*building));

			if (building->descr().type() == MapObjectType::CONSTRUCTIONSITE) {
				upcast(PartiallyFinishedBuilding const, pfb, building);
				fw.c_string((*pfb->m_building).name().c_str());
				fw.unsigned_8(kTypeConstructionSite);

			} else if (building->descr().type() == MapObjectType::DISMANTLESITE) {
				upcast(PartiallyFinishedBuilding const, pfb, building);
				fw.c_string((*pfb->m_building).name().c_str());
				fw.unsigned_8(kTypeDismantleSite);

			} else {
				fw.c_string(building->descr().name().c_str());
				fw.unsigned_8(kTypeBuilding);
			}

			write_priorities(*building, fw);
		} else
			fw.unsigned_8(0);
	}

	fw.write(fs, "binary/building");
	// DONE
}


void MapBuildingPacket::write_priorities
	(const Building & building, FileWrite & fw)
{
	// Used to be base_priority which is no longer used. Remove after b20.
	fw.unsigned_32(0);

	std::map<int32_t, std::map<WareIndex, int32_t> > type_to_priorities;
	std::map<int32_t, std::map<WareIndex, int32_t> >::iterator it;

	const TribeDescr & tribe = building.owner().tribe();
	building.collect_priorities(type_to_priorities);
	for (it = type_to_priorities.begin(); it != type_to_priorities.end(); ++it)
	{
		if (it->second.empty())
			continue;

		// write ware type and priority count
		const int32_t ware_type = it->first;
		fw.unsigned_8(ware_type);
		fw.unsigned_8(it->second.size());

		std::map<WareIndex, int32_t>::iterator it2;
		for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			std::string name;
			WareIndex const ware_index = it2->first;
			if (wwWARE == ware_type)
				name = tribe.get_ware_descr(ware_index)->name();
			else if (wwWORKER == ware_type)
				name = tribe.get_worker_descr(ware_index)->name();
			else
				throw GameDataError
						("unrecognized ware type %d while writing priorities",
						 ware_type);

			fw.c_string(name.c_str());
			fw.unsigned_32(it2->second);
		}
	}

	// write 0xff so the end can be easily identified
	fw.unsigned_8(0xff);
}

void MapBuildingPacket::read_priorities
	(Building & building, FileRead & fr)
{
	fr.unsigned_32(); // unused, was base_priority which is unused. Remove after b20.

	const TribeDescr & tribe = building.owner().tribe();
	Widelands::WareIndex ware_type = INVALID_INDEX;
	// read ware type
	while (0xff != (ware_type = fr.unsigned_8())) {
		// read count of priorities assigned for this ware type
		const uint8_t count = fr.unsigned_8();
		for (uint8_t i = 0; i < count; ++i) {
			WareIndex idx;
			if (wwWARE == ware_type)
				idx = tribe.safe_ware_index(fr.c_string());
			else if (wwWORKER == ware_type)
				idx = tribe.safe_worker_index(fr.c_string());
			else
				throw GameDataError
						("unrecognized ware type %d while reading priorities",
						 ware_type);

			building.set_priority(ware_type, idx, fr.unsigned_32());
		}
	}
}

}
