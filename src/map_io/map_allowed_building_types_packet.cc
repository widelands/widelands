/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "map_io/map_allowed_building_types_packet.h"

#include "base/log.h"
#include "base/macros.h"
#include "io/profile.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"

namespace Widelands {

constexpr int32_t kCurrentPacketVersion = 1;

void MapAllowedBuildingTypesPacket::read(FileSystem& fs,
                                         EditorGameBase& egbase,
                                         bool const skip,
                                         MapObjectLoader&) {
	if (skip) {
		return;
	}

	Profile prof;
	try {
		prof.read("allowed_building_types", nullptr, fs);
	} catch (const WException&) {
		try {
			prof.read("allowed_buildings", nullptr, fs);
		} catch (...) {
			return;
		}
	} catch (...) {
		return;
	}
	try {
		int32_t const packet_version = prof.get_safe_section("global").get_safe_int("packet_version");
		if (packet_version == kCurrentPacketVersion) {
			PlayerNumber const nr_players = egbase.map().get_nrplayers();
			upcast(Game const, game, &egbase);

			//  Now read all players and buildings.
			iterate_players_existing(p, nr_players, egbase, player) {

				const TribeDescr& tribe = player->tribe();
				//  All building types default to false in the game (not in the
				//  editor).
				if (game) {
					for (DescriptionIndex i = 0; i < game->descriptions().nr_buildings(); ++i) {
						player->allow_building_type(i, false);
					}
				}
				try {
					Section& s =
					   prof.get_safe_section(format("player_%u", static_cast<unsigned int>(p)));

					bool allowed;
					while (const char* const name = s.get_next_bool(nullptr, &allowed)) {
						try {
							const DescriptionIndex index = tribe.safe_building_index(name);
							player->allow_building_type(index, allowed);
						} catch (const GameDataError&) {
							log_warn("MapAllowedBuildingTypesPacket - tribe %s does not define "
							         "building type \"%s\"\n",
							         tribe.name().c_str(), name);
						}
					}
				} catch (const WException& e) {
					throw GameDataError("player %u (%s): %s", p, tribe.name().c_str(), e.what());
				}
			}
		} else {
			throw UnhandledVersionError(
			   "MapAllowedBuildingTypesPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("allowed buildings: %s", e.what());
	}
}

void MapAllowedBuildingTypesPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&) {
	Profile prof;
	prof.create_section("global").set_int("packet_version", kCurrentPacketVersion);

	PlayerNumber const nr_players = egbase.map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, egbase, player) {
		const TribeDescr& tribe = player->tribe();
		const std::string section_key = format("player_%u", static_cast<unsigned int>(p));
		Section& section = prof.create_section(section_key.c_str());

		//  Write for all buildings if it is enabled.
		for (const Widelands::DescriptionIndex& building_index : tribe.buildings()) {
			if (player->is_building_type_allowed(building_index)) {
				const BuildingDescr* building_descr =
				   egbase.descriptions().get_building_descr(building_index);
				section.set_bool(building_descr->name().c_str(), true);
			}
		}
	}

	prof.write("allowed_building_types", false, fs);
}
}  // namespace Widelands
