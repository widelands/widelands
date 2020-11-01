/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "map_io/map_elemental_packet.h"

#include <boost/algorithm/string.hpp>

#include "io/profile.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"

namespace Widelands {

constexpr int32_t kEightPlayersPacketVersion = 1;
constexpr int32_t kSixteenPlayersPacketVersion = 2;

void MapElementalPacket::pre_read(FileSystem& fs, Map* map) {
	Profile prof;
	prof.read("elemental", nullptr, fs);
	Section& s = prof.get_safe_section("global");

	try {
		int32_t const packet_version = s.get_int("packet_version");
		if (packet_version >= kEightPlayersPacketVersion &&
		    packet_version <= kSixteenPlayersPacketVersion) {
			map->width_ = s.get_int("map_w");
			map->height_ = s.get_int("map_h");
			map->set_nrplayers(s.get_int("nr_players"));
			map->set_name(s.get_string("name"));
			map->set_author(s.get_string("author"));
			map->set_description(s.get_string("descr"));
			map->set_hint(s.get_string("hint", ""));
			map->set_background(s.get_string("background", ""));
			old_world_name_ = s.get_string("world", "");
			map->set_background_theme(s.get_string(
			   "theme", old_world_name_.empty() ?
			               "" :
			               Map::get_old_world_info_by_old_name(old_world_name_).name.c_str()));

			std::string t = s.get_string("tags", "");
			if (!t.empty()) {
				std::vector<std::string> tags;
				boost::split(tags, t, boost::is_any_of(","));

				for (std::vector<std::string>::const_iterator ci = tags.begin(); ci != tags.end();
				     ++ci) {
					std::string tn = *ci;
					boost::trim(tn);
					map->add_tag(tn);
				}
			}

			// Get suggested teams
			map->suggested_teams_.clear();

			uint16_t team_section_id = 0;
			std::string teamsection_key = (boost::format("teams%02i") % team_section_id).str();
			while (Section* teamsection = prof.get_section(teamsection_key)) {

				// A lineup is made up of teams
				SuggestedTeamLineup lineup;

				uint16_t team_number = 1;
				std::string team_key = (boost::format("team%i") % team_number).str();
				std::string team_string = teamsection->get_string(team_key.c_str(), "");
				while (!team_string.empty()) {
					// A team is made up of players
					SuggestedTeam team;

					std::vector<std::string> players_string;
					boost::split(players_string, team_string, boost::is_any_of(","));

					for (const std::string& player : players_string) {
						PlayerNumber player_number =
						   static_cast<PlayerNumber>(boost::lexical_cast<unsigned int>(player.c_str()));
						assert(player_number < kMaxPlayers);
						team.push_back(player_number);
					}

					lineup.push_back(team);

					// Increase team number
					++team_number;
					team_key = (boost::format("team%i") % team_number).str();
					team_string = teamsection->get_string(team_key.c_str(), "");
				}

				map->suggested_teams_.push_back(lineup);

				// Increase teamsection
				++team_section_id;
				teamsection_key = (boost::format("teams%02i") % team_section_id).str();
			}
		} else {
			throw UnhandledVersionError(
			   "MapElementalPacket", packet_version, kEightPlayersPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("elemental data: %s", e.what());
	}
}

void MapElementalPacket::read(FileSystem& fs, EditorGameBase& egbase, bool, MapObjectLoader&) {
	pre_read(fs, egbase.mutable_map());
}

void MapElementalPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&) {

	Profile prof;
	Section& global_section = prof.create_section("global");

	const Map& map = egbase.map();
	Widelands::PlayerNumber nr_players = map.get_nrplayers();

	// Maps with more than 8 players won't be compatible with older versions of Widelands.
	// The packet format itself hasn't changed, so we always want to allow loading maps with <= 8
	// players.
	global_section.set_int("packet_version", nr_players <= 8 ? kEightPlayersPacketVersion :
	                                                           kSixteenPlayersPacketVersion);
	global_section.set_int("map_w", map.get_width());
	global_section.set_int("map_h", map.get_height());
	global_section.set_int("nr_players", nr_players);
	global_section.set_string("name", map.get_name());
	global_section.set_string("author", map.get_author());
	global_section.set_string("descr", map.get_description());
	global_section.set_string("hint", map.get_hint());
	if (!map.get_background().empty()) {
		global_section.set_string("background", map.get_background());
	}
	if (!map.get_background_theme().empty()) {
		global_section.set_string("theme", map.get_background_theme());
	}
	global_section.set_string("tags", boost::algorithm::join(map.get_tags(), ","));

	int counter = 0;
	for (Widelands::SuggestedTeamLineup lineup : map.get_suggested_teams()) {
		Section& teams_section =
		   prof.create_section((boost::format("teams%02d") % counter++).str().c_str());
		int lineup_counter = 0;
		for (Widelands::SuggestedTeam team : lineup) {
			std::string section_contents;
			for (std::vector<PlayerNumber>::const_iterator it = team.begin(); it != team.end(); ++it) {
				if (it == team.begin()) {
					section_contents = (boost::format("%d") % static_cast<unsigned int>(*it)).str();
				} else {
					section_contents =
					   (boost::format("%s,%d") % section_contents % static_cast<unsigned int>(*it))
					      .str();
				}
			}
			teams_section.set_string(
			   (boost::format("team%d") % ++lineup_counter).str().c_str(), section_contents);
		}
	}

	prof.write("elemental", false, fs);
}
}  // namespace Widelands
