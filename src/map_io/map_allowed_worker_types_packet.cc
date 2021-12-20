/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "map_io/map_allowed_worker_types_packet.h"

#include "io/profile.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"

namespace Widelands {

constexpr int32_t kCurrentPacketVersion = 1;

void MapAllowedWorkerTypesPacket::read(FileSystem& fs,
                                       EditorGameBase& egbase,
                                       bool skip,
                                       MapObjectLoader&) {
	if (skip) {
		return;
	}

	// Worker types that the tribe has are allowed by default - this is to make sure that old maps
	// remain playable without change even if new worker types are introduced.
	// Worker types that the tribe doesn't have are always disallowed.
	// If our file is not there, there is nothing to be done.
	Profile prof;
	try {
		prof.read("allowed_worker_types", nullptr, fs);
	} catch (...) {
		return;
	}

	try {
		const int32_t packet_version = prof.get_safe_section("global").get_safe_int("packet_version");
		if (packet_version == kCurrentPacketVersion) {
			iterate_players_existing(p, egbase.map().get_nrplayers(), egbase, player) {
				const TribeDescr& tribe = player->tribe();
				try {
					Section* s = prof.get_section(format("player_%u", static_cast<unsigned int>(p)));
					if (s == nullptr) {
						continue;
					}

					// Only allow workers that the player's tribe has.
					for (size_t i = 0; i < egbase.descriptions().nr_workers(); ++i) {
						const DescriptionIndex& worker_index = static_cast<DescriptionIndex>(i);
						const WorkerDescr& worker_descr =
						   *egbase.descriptions().get_worker_descr(worker_index);
						if (worker_descr.is_buildable() && player->tribe().has_worker(worker_index)) {
							player->allow_worker_type(
							   worker_index, s->get_bool(worker_descr.name().c_str(), true));
						} else {
							player->allow_worker_type(worker_index, false);
						}
					}
				} catch (const WException& e) {
					throw GameDataError("player %u (%s): %s", p, tribe.name().c_str(), e.what());
				}
			}
		} else {
			throw UnhandledVersionError(
			   "MapAllowedWorkerTypesPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("allowed worker types: %s", e.what());
	}
}

void MapAllowedWorkerTypesPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&) {
	Profile prof;
	prof.create_section("global").set_int("packet_version", kCurrentPacketVersion);

	bool forbidden_worker_seen = false;
	iterate_players_existing_const(p, egbase.map().get_nrplayers(), egbase, player) {
		const TribeDescr& tribe = player->tribe();
		const std::string section_key = format("player_%u", static_cast<unsigned int>(p));
		Section& section = prof.create_section(section_key.c_str());

		// Only write the workers which are disabled.
		for (const DescriptionIndex& worker_index : tribe.workers()) {
			if (!player->is_worker_type_allowed(worker_index)) {
				const WorkerDescr* worker_descr = egbase.descriptions().get_worker_descr(worker_index);
				section.set_bool(worker_descr->name().c_str(), false);
				forbidden_worker_seen = true;
			}
		}
	}

	// Only write this package if there is a forbidden worker type.
	if (forbidden_worker_seen) {
		prof.write("allowed_worker_types", false, fs);
	}
}
}  // namespace Widelands
