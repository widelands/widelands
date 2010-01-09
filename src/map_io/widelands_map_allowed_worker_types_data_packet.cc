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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands_map_allowed_worker_types_data_packet.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "logic/tribe.h"

#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Allowed_Worker_Types_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip)
		return;

	Player_Number const nr_players = egbase.map().get_nrplayers();
	Profile prof;
	try {
		prof.read("allowed_worker_types", 0, fs);
	} catch (...) {
		//  Could not read data, must allow buildable worker types for players.
		iterate_players_existing(p, nr_players, egbase, player) {
			Tribe_Descr const & tribe = player->tribe();
			for (Ware_Index i = tribe.get_nrworkers(); Ware_Index::First() < i;)
				if (tribe.get_worker_descr(--i)->is_buildable())
					player->allow_worker_type(i, true);
		}
		return;
	}
	try {
		int32_t const packet_version =
			prof.get_safe_section("global").get_safe_int("packet_version");
		if (packet_version == CURRENT_PACKET_VERSION) {
			iterate_players_existing(p, nr_players, egbase, player) {
				Tribe_Descr const & tribe = player->tribe();
				char buffer[10];
				snprintf(buffer, sizeof(buffer), "player_%u", p);
				try {
					Section & s = prof.get_safe_section(buffer);
					Ware_Index const nr_workers = tribe.get_nrworkers();
					for (Ware_Index w = Ware_Index::First(); w < nr_workers; ++w) {
						Worker_Descr const & w_descr = *tribe.get_worker_descr(w);
						if (w_descr.is_buildable())
							player->allow_worker_type
								(w, s.get_bool(w_descr.name().c_str(), false));
					}
				} catch (_wexception const & e) {
					throw game_data_error
						("player %u (%s): %s", p, tribe.name().c_str(), e.what());
				}
			}
		} else
			throw game_data_error
				(_("unknown/unhandled version %i"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("allowed worker types: %s"), e.what());
	}
}


void Map_Allowed_Worker_Types_Data_Packet::Write
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

		//  Write for all workers if it is enabled.
		Ware_Index const nr_workers = tribe.get_nrworkers();
		for (Ware_Index b = Ware_Index::First(); b < nr_workers; ++b)
			if (player->is_worker_type_allowed(b))
				section.set_bool
					(tribe.get_worker_descr(b)->name().c_str(), true);
	}

	prof.write("allowed_worker_types", false, fs);
}

}
