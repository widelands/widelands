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

#include "map_io/widelands_map_allowed_building_types_data_packet.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "profile/profile.h"
#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Allowed_Building_Types_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader &)
{
	if (skip)
		return;

	Profile prof;
	try {
		prof.read("allowed_building_types", nullptr, fs);
	} catch (const _wexception &) {
		try {
			prof.read("allowed_buildings", nullptr, fs);
		} catch (...) {
			return;
		}
	} catch (...) {
		return;
	}
	try {
		int32_t const packet_version =
			prof.get_safe_section("global").get_safe_int("packet_version");
		if (packet_version == CURRENT_PACKET_VERSION) {
			Player_Number const nr_players = egbase.map().get_nrplayers();
			upcast(Game const, game, &egbase);

			//  Now read all players and buildings.
			iterate_players_existing(p, nr_players, egbase, player) {
				const Tribe_Descr & tribe = player->tribe();
				//  All building types default to false in the game (not in the
				//  editor).
				if (game)
					for
						(Building_Index i = tribe.get_nrbuildings();
						 Building_Index::First() < i;)
						player->allow_building_type(--i, false);
				char buffer[10];
				snprintf(buffer, sizeof(buffer), "player_%u", p);
				try {
					Section & s = prof.get_safe_section(buffer);

					bool allowed;
					while (const char * const name = s.get_next_bool(nullptr, &allowed)) {
						if (Building_Index const index = tribe.building_index(name))
							player->allow_building_type(index, allowed);
						else
							throw game_data_error
								("tribe %s does not define building type \"%s\"",
								 tribe.name().c_str(), name);
					}
				} catch (const _wexception & e) {
					throw game_data_error
						("player %u (%s): %s", p, tribe.name().c_str(), e.what());
				}
			}
		} else
			throw game_data_error
				("unknown/unhandled version %i", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("allowed buildings: %s", e.what());
	}
}


void Map_Allowed_Building_Types_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
{
	Profile prof;
	prof.create_section("global").set_int
		("packet_version", CURRENT_PACKET_VERSION);

	Player_Number const nr_players = egbase.map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, egbase, player) {
		const Tribe_Descr & tribe = player->tribe();
		char buffer[10];
		snprintf(buffer, sizeof(buffer), "player_%u", p);
		Section & section = prof.create_section(buffer);

		//  Write for all buildings if it is enabled.
		Building_Index const nr_buildings = tribe.get_nrbuildings();
		for (Building_Index b = Building_Index::First(); b < nr_buildings; ++b)
			if (player->is_building_type_allowed(b))
				section.set_bool
					(tribe.get_building_descr(b)->name().c_str(), true);
	}

	prof.write("allowed_building_types", false, fs);
}

}
