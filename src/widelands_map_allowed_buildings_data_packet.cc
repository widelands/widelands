/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "editorinteractive.h"
#include "editor_game_base.h"
#include "filesystem.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "profile.h"
#include "tribe.h"
#include "widelands_map_data_packet_ids.h"
#include "world.h"


#define CURRENT_PACKET_VERSION 1


Widelands_Map_Allowed_Buildings_Data_Packet::
~Widelands_Map_Allowed_Buildings_Data_Packet
()
{}


void Widelands_Map_Allowed_Buildings_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   if (skip)
      return;

   Profile prof;
   try {
      prof.read("allowed_buildings", 0, fs);
	} catch (...) {
      // Packet wasn't save. Same as skip
      return;
	}
   Section* s = prof.get_section("global");

	const int32_t packet_version = s->get_int("packet_version");
	if (packet_version == CURRENT_PACKET_VERSION) {
		const Player_Number nr_players = egbase->map().get_nrplayers();
      // First of all: if we shouldn't skip, all buildings default to false in the game (!not editor)
		if (dynamic_cast<Game *>(egbase))
			iterate_players_existing(p, nr_players, *egbase, player) {
				const int32_t nr_buildings = player->tribe().get_nrbuildings();
				for (int32_t b = 0; b < nr_buildings; ++b)
					player->allow_building(b, false);
			}

      // Now read all players and buildings
		iterate_players_existing(p, nr_players, *egbase, player) {
			const Tribe_Descr & tribe = player->tribe();
			char buffer[10];
			snprintf(buffer, sizeof(buffer), "player_%u", p);
         s = prof.get_safe_section(buffer);

         // Write for all buildings if it is enabled
				while (const char * const name = s->get_next_bool(0, 0)) {
            bool allowed = s->get_bool(name);
					const int32_t index = tribe.get_building_index(name);
					if (index == -1)
						throw wexception
							("Unknown building found in map (Allowed_Buildings_Data): "
							 "%s is not in tribe %s",
							 name, tribe.name().c_str());
					player->allow_building(index, allowed);
			}
		}
	} else
		throw wexception
			("Unknown version %i Allowed_Building_Data_Packet in map!",
			 packet_version);
}


/*
 * Write Function
 */
void Widelands_Map_Allowed_Buildings_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   Profile prof;
	prof.create_section("global")
		->set_int("packet_version", CURRENT_PACKET_VERSION);

	const Player_Number nr_players = egbase->map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, *egbase, player) {
		const Tribe_Descr & tribe = player->tribe();
			char buffer[10];
		snprintf(buffer, sizeof(buffer), "player_%u", p);
			Section & section = *prof.create_section(buffer);

      // Write for all buildings if it is enabled
			const Building_Descr::Index nr_buildings = tribe.get_nrbuildings();
		for (Building_Descr::Index b = 0; b < nr_buildings; ++b)
			section.set_bool
				(tribe.get_building_descr(b)->name().c_str(),
				 player->is_building_allowed(b));
	}

   prof.write("allowed_buildings", false, fs);
   // Done
}
