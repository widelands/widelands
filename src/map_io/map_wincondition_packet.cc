/*
 * Copyright (C) 2019-2022 by the Widelands Development Team
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

#include "map_io/map_wincondition_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"

namespace Widelands {

constexpr uint8_t kCurrentPacketVersion = 1;

void MapWinconditionPacket::read(FileSystem& fs, Map& map) {
	if (!fs.file_exists("binary/wincondition")) {
		// This can be empty when win conditions don't need any special map data
		return;
	}

	try {
		FileRead fr;
		fr.open(fs, "binary/wincondition");

		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			const size_t no_of_fields = fr.unsigned_32();
			std::set<FCoords>* fields = map.mutable_valuable_fields();
			fields->clear();

			for (size_t i = 0; i < no_of_fields; ++i) {
				const int32_t x = fr.signed_16();
				const int32_t y = fr.signed_16();
				fields->insert(map.get_fcoords(Coords(x, y)));
			}
		} else {
			throw UnhandledVersionError(
			   "MapWinconditionPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("win condition data: %s", e.what());
	}
}

void MapWinconditionPacket::write(FileSystem& fs, Map& map) {
	// We only write this packet if we have something interesting to write to it.
	std::set<FCoords>& fields = *map.mutable_valuable_fields();
	if (!fields.empty()) {
		FileWrite fw;
		fw.unsigned_8(kCurrentPacketVersion);

		fw.unsigned_32(fields.size());
		for (const auto& field : fields) {
			fw.signed_16(field.x);
			fw.signed_16(field.y);
		}

		fw.write(fs, "binary/wincondition");
	}
}
}  // namespace Widelands
