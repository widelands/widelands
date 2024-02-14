/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "map_io/map_building_packet.h"

#include "base/macros.h"
#include "economy/request.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 4;  // since v1.0

// constants to handle special building types
constexpr uint8_t kTypeBuilding = 0;
constexpr uint8_t kTypeConstructionSite = 1;
constexpr uint8_t kTypeDismantleSite = 2;

void MapBuildingPacket::read(FileSystem& fs,
                             EditorGameBase& egbase,
                             bool const skip,
                             MapObjectLoader& mol) {
	if (skip) {
		return;
	}
	FileRead fr;
	try {
		fr.open(fs, "binary/building");
	} catch (...) {
		return;
	}
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version >= 4 && packet_version <= kCurrentPacketVersion) {
			const Map& map = egbase.map();
			uint16_t const width = map.get_width();
			uint16_t const height = map.get_height();
			FCoords c;
			for (c.y = 0; c.y < height; ++c.y) {
				for (c.x = 0; c.x < width; ++c.x) {
					if (fr.unsigned_8() != 0u) {
						PlayerNumber const p = fr.unsigned_8();
						Serial const serial = fr.unsigned_32();
						char const* const name = fr.c_string();
						uint8_t const building_type = fr.unsigned_8();

						//  No building lives on more than one main place.

						//  Get the tribe and the building index.
						if (Player* const player = egbase.get_safe_player(p)) {
							const TribeDescr& tribe = player->tribe();
							const DescriptionIndex index =
							   egbase.mutable_descriptions()->load_building(name);
							const BuildingDescr* bd = tribe.get_building_descr(index);
							// Check if tribe has this building itself
							// OR alternatively if this building might be a conquered militarysite
							if (!tribe.has_building(index) &&
							    !((bd != nullptr) && bd->type() == MapObjectType::MILITARYSITE)) {
								throw GameDataError("tribe %s does not define building type \"%s\"",
								                    tribe.name().c_str(),
								                    bd != nullptr ? bd->name().c_str() : name);
							}

							//  Now, create this Building, take extra special care for
							//  constructionsites. All data is read later.
							Building* building;
							if (building_type == kTypeConstructionSite) {
								building = &egbase.warp_constructionsite(c, p, index, true);
							} else if (building_type == kTypeDismantleSite) {
								building = &egbase.warp_dismantlesite(c, p, true, {{index, true}});
							} else {
								building = &egbase.warp_building(c, p, index);
							}

							mol.register_object<Building>(serial, *building);
						} else {
							throw GameDataError("player %u does not exist", p);
						}
					}
				}
			}
		} else {
			throw UnhandledVersionError("MapBuildingPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("buildings: %s", e.what());
	}
}

/*
 * Write Function
 */
void MapBuildingPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver& mos) {
	FileWrite fw;

	// now packet version
	fw.unsigned_16(kCurrentPacketVersion);

	// Write buildings and owner, register this with the map_object_saver so that
	// it's data can be saved later.
	const Map& map = egbase.map();
	Extent const extent = map.extent();
	iterate_Map_FCoords(map, extent, fc) {
		upcast(Building const, building, fc.field->get_immovable());
		if ((building != nullptr) && building->get_position() == fc) {
			//  We only write Buildings.
			//  Buildings can life on only one main position.
			assert(!mos.is_object_known(*building));

			fw.unsigned_8(1);
			fw.unsigned_8(building->owner().player_number());
			fw.unsigned_32(mos.register_object(*building));

			if (building->descr().type() == MapObjectType::CONSTRUCTIONSITE) {
				upcast(PartiallyFinishedBuilding const, pfb, building);
				fw.c_string((*pfb->building_).name().c_str());
				fw.unsigned_8(kTypeConstructionSite);

			} else if (building->descr().type() == MapObjectType::DISMANTLESITE) {
				upcast(PartiallyFinishedBuilding const, pfb, building);
				fw.c_string((*pfb->building_).name().c_str());
				fw.unsigned_8(kTypeDismantleSite);

			} else {
				fw.c_string(building->descr().name().c_str());
				fw.unsigned_8(kTypeBuilding);
			}
		} else {
			fw.unsigned_8(0);
		}
	}

	fw.write(fs, "binary/building");
	// DONE
}

}  // namespace Widelands
