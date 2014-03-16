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

#include "map_io/widelands_map_allowed_worker_types_data_packet.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "profile/profile.h"
#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Allowed_Worker_Types_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    skip,
	 Map_Map_Object_Loader &)
{
	if (skip)
		return;

	// Worker types are allowed by default - this is to make sure that old maps
	// remain playable without change even if new worker types are introduced. If
	// our file is not there, there is nothing to be done.
	Profile prof;
	try {
		prof.read("allowed_worker_types", nullptr, fs);
	} catch (...) {
		return;
	}

	try {
		const int32_t packet_version =
			prof.get_safe_section("global").get_safe_int("packet_version");
		if (packet_version == CURRENT_PACKET_VERSION) {
			iterate_players_existing(p, egbase.map().get_nrplayers(), egbase, player) {
				const Tribe_Descr & tribe = player->tribe();
				char buffer[10];
				snprintf(buffer, sizeof(buffer), "player_%u", p);
				try {
					Section* s = prof.get_section(buffer);
					if (s == nullptr)
						continue;

					for (Ware_Index w = Ware_Index::First(); w < tribe.get_nrworkers(); ++w) {
						const Worker_Descr & w_descr = *tribe.get_worker_descr(w);
						if (w_descr.is_buildable())
							player->allow_worker_type
								(w, s->get_bool(w_descr.name().c_str(), true));
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
		throw game_data_error("allowed worker types: %s", e.what());
	}
}


void Map_Allowed_Worker_Types_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
{
	Profile prof;
	prof.create_section("global").set_int
		("packet_version", CURRENT_PACKET_VERSION);

	bool forbidden_worker_seen = false;
	iterate_players_existing_const(p, egbase.map().get_nrplayers(), egbase, player) {
		const Tribe_Descr & tribe = player->tribe();
		char buffer[10];
		snprintf(buffer, sizeof(buffer), "player_%u", p);
		Section & section = prof.create_section(buffer);

		// Only write the workers which are disabled.
		for (Ware_Index b = Ware_Index::First(); b < tribe.get_nrworkers(); ++b) {
			if (!player->is_worker_type_allowed(b)) {
				section.set_bool(tribe.get_worker_descr(b)->name().c_str(), false);
				forbidden_worker_seen = true;
			}
		}
	}

	// Only write this package if there is a forbidden worker type.
	if (forbidden_worker_seen) {
		prof.write("allowed_worker_types", false, fs);
	}
}

}
