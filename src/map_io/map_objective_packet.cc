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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "map_io/map_objective_packet.h"

#include <memory>

#include "io/profile.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/objective.h"

namespace Widelands {

constexpr int32_t kCurrentPacketVersion = 2;

void read_objective_data(FileSystem& fs, EditorGameBase& egbase) {
	Profile prof;
	try {
		prof.read("objective", nullptr, fs);
	} catch (...) {
		return;
	}
	try {
		int32_t const packet_version = prof.get_safe_section("global").get_safe_int("packet_version");
		if (packet_version == kCurrentPacketVersion) {
			while (Section* const s = prof.get_next_section(nullptr)) {
				char const* const name = s->get_name();
				try {
					std::unique_ptr<Objective> objective(new Objective(name));
					Map::Objectives* objectives = egbase.mutable_map()->mutable_objectives();
					if (objectives->count(name)) {
						throw GameDataError("duplicated");
					}
					objective->set_descname(s->get_string("name", name));
					objective->set_descr(s->get_safe_string("descr"));
					objective->set_visible(s->get_safe_bool("visible"));
					objective->set_done(s->get_bool("done", false));
					objectives->insert(std::make_pair(name, std::move(objective)));
				} catch (const WException& e) {
					throw GameDataError("%s: %s", name, e.what());
				}
			}
		} else {
			throw UnhandledVersionError("MapObjectivePacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("Objectives: %s", e.what());
	}
}

void write_objective_data(FileSystem& fs, EditorGameBase& egbase) {
	Profile prof;
	prof.create_section("global").set_int("packet_version", kCurrentPacketVersion);

	for (const auto& item : egbase.map().objectives()) {
		Section& s = prof.create_section(item.second->name().c_str());
		s.set_string("name", item.second->descname());
		s.set_string("descr", item.second->descr());
		s.set_bool("visible", item.second->visible());
		s.set_bool("done", item.second->done());
	}

	prof.write("objective", false, fs);
}
}  // namespace Widelands
