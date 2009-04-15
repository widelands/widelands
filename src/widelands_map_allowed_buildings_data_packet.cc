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

#include "widelands_map_allowed_buildings_data_packet.h"

#include "game.h"
#include "player.h"
#include "profile.h"
#include "tribe.h"
#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Allowed_Buildings_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip)
		return;

	Profile prof;
	try {prof.read("allowed_buildings", 0, fs);} catch (...) {return;}
	try {
		int32_t const packet_version =
			prof.get_safe_section("global").get_safe_int("packet_version");
		if (packet_version == CURRENT_PACKET_VERSION) {
			Player_Number const nr_players = egbase.map().get_nrplayers();
			upcast(Game const, game, &egbase);

			//  Now read all players and buildings.
			iterate_players_existing(p, nr_players, egbase, player) {
				Tribe_Descr const & tribe = player->tribe();
				//  All building types default to false in the game (not in the
				//  editor).
				if (game)
					for
						(Building_Index i = tribe.get_nrbuildings();
						 Building_Index::First() < i;)
						player->allow_building(--i, false);
				char buffer[10];
				snprintf(buffer, sizeof(buffer), "player_%u", p);
				try {
					Section & s = prof.get_safe_section(buffer);

					//  Write for all buildings if it is enabled.
					bool allowed;
					while (const char * const name = s.get_next_bool(0, &allowed)) {
						if (Building_Index const index = tribe.building_index(name))
							player->allow_building(index, allowed);
						else
							throw wexception
								("tribe %s does not define building type \"%s\"",
								 tribe.name().c_str(), name);
					}
				} catch (_wexception const & e) {
					throw wexception
						("player %u (%s): %s", p, tribe.name().c_str(), e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (_wexception const & e) {
		throw wexception("Allowed buildings: %s", e.what());
	}
}


void Map_Allowed_Buildings_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver * const)
throw (_wexception)
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
			if (player->is_building_allowed(b))
				section.set_bool
					(tribe.get_building_descr(b)->name().c_str(), true);
	}

	prof.write("allowed_buildings", false, fs);
}

}; // namespace
