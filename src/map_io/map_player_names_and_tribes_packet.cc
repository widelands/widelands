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

#include "map_io/map_player_names_and_tribes_packet.h"

#include "base/string.h"
#include "io/profile.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/player.h"

namespace Widelands {

constexpr int32_t kCurrentPacketVersion = 2;

/*
 * Read Function
 *
 * this is a scenario packet, it might be that we have to skip it
 */
void MapPlayerNamesAndTribesPacket::read(FileSystem& fs,
                                         EditorGameBase& egbase,
                                         bool const skip,
                                         MapObjectLoader& /* mol */) {
	pre_read(fs, egbase.mutable_map(), skip);
}

void MapPlayerNamesAndTribesPacket::pre_read(FileSystem& fs, Map* const map, bool const skip) {
	if (skip) {
		return;
	}

	Profile prof;
	prof.read("player_names", nullptr, fs);

	try {
		int32_t const packet_version = prof.get_safe_section("global").get_int("packet_version");
		// Supporting older versions for map loading
		if (1 <= packet_version && packet_version <= kCurrentPacketVersion) {
			i18n::Textdomain td("widelands");
			PlayerNumber const nr_players = map->get_nrplayers();
			iterate_player_numbers(p, nr_players) {
				Section& s = prof.get_safe_section(format("player_%u", static_cast<unsigned int>(p)));

				// Replace empty or standard player names with localized standard player name
				std::string player_name = s.get_string("name", "");
				if (player_name.empty() ||
				    player_name == format("Player %u", static_cast<unsigned int>(p))) {
					player_name = format(_("Player %u"), static_cast<unsigned int>(p));
				}
				map->set_scenario_player_name(p, player_name);
				map->set_scenario_player_tribe(p, s.get_string("tribe", ""));
				map->set_scenario_player_ai(p, s.get_string("ai", ""));
				map->set_scenario_player_closeable(p, s.get_bool("closeable", false));
			}
		} else {
			throw UnhandledVersionError(
			   "MapPlayerNamesAndTribesPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("player names and tribes: %s", e.what());
	}
}

void MapPlayerNamesAndTribesPacket::write(FileSystem& fs,
                                          EditorGameBase& egbase,
                                          MapObjectSaver& /* mos */) {
	Profile prof;

	prof.create_section("global").set_int("packet_version", kCurrentPacketVersion);

	i18n::Textdomain td("widelands");
	const Map& map = egbase.map();
	PlayerNumber const nr_players = map.get_nrplayers();
	iterate_player_numbers(p, nr_players) {
		const std::string section_key = format("player_%u", static_cast<unsigned int>(p));

		// Make sure that no player name is empty, and trim leading/trailing whitespaces.
		std::string player_name = map.get_scenario_player_name(p);
		trim(player_name);

		// Save default player names as empty
		if (player_name == format(_("Player %u"), static_cast<unsigned int>(p))) {
			player_name = "";
		}

		Section& s = prof.create_section(section_key.c_str());
		s.set_string("name", player_name);
		s.set_string("tribe", map.get_scenario_player_tribe(p));
		s.set_string("ai", map.get_scenario_player_ai(p));
		s.set_bool("closeable", map.get_scenario_player_closeable(p));
		// Only read by multiplayer loadgame, transfered over from the GamePlayerInfoPacket
		s.set_int("team", egbase.get_player(p) ? egbase.get_player(p)->team_number() : 0);
		s.set_bool("random", egbase.get_player(p) ? egbase.get_player(p)->has_random_tribe() : false);
	}

	prof.write("player_names", false, fs);
}
}  // namespace Widelands
