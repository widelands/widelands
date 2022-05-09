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

#include "game_io/game_class_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 8;

void GameClassPacket::read(FileSystem& fs, Game& game, MapObjectLoader* /* mol */) {
	try {
		FileRead fr;
		fr.open(fs, "binary/game_class");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version <= kCurrentPacketVersion && packet_version >= 4) {
			game.gametime_ = Time(fr);
			game.scenario_difficulty_ = fr.unsigned_32();
			// TODO(Nordfriese): Savegame compatibility
			if (packet_version >= 5 && fr.unsigned_8() == 0) {
				game.set_write_replay(false);
			}

			game.list_of_scenarios_.clear();
			if (packet_version >= 6) {
				for (size_t i = fr.unsigned_32(); i > 0; --i) {
					game.list_of_scenarios_.push_back(fr.string());
				}
			}

			if (packet_version >= 7) {
				/* This design is the fix for bug #4786. The order in which units are loaded is
				 * stored in the savegame to allow recreating it exactly during loading without
				 * having to always load all tribes. Otherwise, we'd get desyncs because the
				 * dynamic load order is different during loading from starting a new game.
				 */
				game.postload_addons_before_loading();
				for (size_t i = fr.unsigned_32(); i > 0; --i) {
					/* This tells the descriptions manager to actually load the description
					 * with the given name, e.g. "barbarians" or "frisians_well".
					 * Takes care of legacy lookup and skipping already loaded units.
					 */
					Notifications::publish(NoteMapObjectDescription(
					   fr.string(), NoteMapObjectDescription::LoadType::kObject));
				}
				game.postload_tribes();
			} else {
				game.check_legacy_addons_desync_magic();
			}

			game.diplomacy_allowed_ = (packet_version < 8 || fr.unsigned_8() > 0);
			game.pending_diplomacy_actions_.clear();
			if (packet_version >= 8) {
				for (size_t i = fr.unsigned_32(); i > 0; --i) {
					const PlayerNumber p1 = fr.unsigned_8();
					const DiplomacyAction a = static_cast<DiplomacyAction>(fr.unsigned_8());
					const PlayerNumber p2 = fr.unsigned_8();
					game.pending_diplomacy_actions_.emplace_back(p1, a, p2);
				}
			}
		} else {
			throw UnhandledVersionError("GameClassPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("game class: %s", e.what());
	}
}

/*
 * Write Function
 */
void GameClassPacket::write(FileSystem& fs, Game& game, MapObjectSaver* const /* mos */) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	// From the interactive player, is saved somewhere else
	// Computer players are saved somewhere else

	// CMD Queue is saved later
	// We do not care for real time.

	// EDITOR GAME CLASS
	// Write gametime
	game.gametime_.save(fw);

	fw.unsigned_32(game.scenario_difficulty_);
	fw.unsigned_8((game.writereplay_ || game.is_replay()) ? 1 : 0);

	fw.unsigned_32(game.list_of_scenarios_.size());
	for (const std::string& s : game.list_of_scenarios_) {
		fw.string(s);
	}

	fw.unsigned_32(game.descriptions().load_order().size());
	for (const std::string& s : game.descriptions().load_order()) {
		fw.string(s);
	}

	fw.unsigned_8(game.diplomacy_allowed_ ? 1 : 0);
	fw.unsigned_32(game.pending_diplomacy_actions_.size());
	for (const auto& a : game.pending_diplomacy_actions_) {
		fw.unsigned_8(a.sender);
		fw.unsigned_8(static_cast<uint8_t>(a.action));
		fw.unsigned_8(a.other);
	}

	// TODO(sirver,trading): save/load trade_agreements and related data.

	// We do not care for players, since they were set
	// on game initialization to match Map::scenario_player_[names|tribes]
	// or vice versa, so this is handled by map loader

	// Objects are loaded and saved by map

	// Tribes and wares are handled by map
	// InteractiveBase doesn't need saving

	// Map is handled by map saving

	// Track pointers are not saved in save games

	fw.write(fs, "binary/game_class");
}
}  // namespace Widelands
