/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "game_io/game_class_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 3;

void GameClassPacket::read
	(FileSystem & fs, Game & game, MapObjectLoader *)
{
	try {
		FileRead fr;
		fr.open(fs, "binary/game_class");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			game.gametime_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw GameDataError("game class: %s", e.what());
	}
}

/*
 * Write Function
 */
void GameClassPacket::write
	(FileSystem & fs, Game & game, MapObjectSaver * const)
{
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	// From the interactive player, is saved somewhere else
	// Computer players are saved somewhere else

	// CMD Queue is saved later
	// We do not care for real time.

	// EDITOR GAME CLASS
	// Write gametime
	fw.unsigned_32(game.gametime_);

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

}
