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

#include "map_io/map_node_ownership_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;

void MapNodeOwnershipPacket::read(FileSystem& fs,
                                  EditorGameBase& egbase,
                                  bool const skip,
                                  MapObjectLoader&)

{
	if (skip) {
		return;
	}
	FileRead fr;
	try {
		fr.open(fs, "binary/node_ownership");
	} catch (...) {
		try {
			fr.open(fs, "binary/owned_fields");
		} catch (...) {
			return;
		}
	}
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const Map& map = egbase.map();
			MapIndex const max_index = map.max_index();
			for (MapIndex i = 0; i < max_index; ++i) {
				map[i].set_owned_by(fr.unsigned_8());
			}
		} else {
			throw UnhandledVersionError(
			   "MapNodeOwnershipPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("ownership: %s", e.what());
	}
}

void MapNodeOwnershipPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const Map& map = egbase.map();
	MapIndex const max_index = map.max_index();
	for (MapIndex i = 0; i < max_index; ++i) {
		fw.unsigned_8(map[i].get_owned_by());
	}

	fw.write(fs, "binary/node_ownership");
}
}  // namespace Widelands
