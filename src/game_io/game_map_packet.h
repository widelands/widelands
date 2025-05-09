/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_GAME_IO_GAME_MAP_PACKET_H
#define WL_GAME_IO_GAME_MAP_PACKET_H

#include "game_io/game_data_packet.h"

namespace Widelands {

struct MapSaver;
struct WidelandsMapLoader;

/*
 * This is just a wrapper around MapSaver and MapLoader
 */
struct GameMapPacket : public GameDataPacket {
	GameMapPacket() = default;
	~GameMapPacket() override;

	/// Ensures that the world gets loaded but does not much more.
	void read(FileSystem&, Game&, MapObjectLoader* = nullptr) override;

	void read_complete(Game&);  ///  Loads the rest of the map.

	void write(FileSystem&, Game&, MapObjectSaver* = nullptr) override;

	MapObjectSaver* get_map_object_saver() {
		return mos_;
	}
	MapObjectLoader* get_map_object_loader() {
		return mol_;
	}

private:
	MapObjectSaver* mos_{nullptr};
	MapObjectLoader* mol_{nullptr};
	MapSaver* wms_{nullptr};
	WidelandsMapLoader* wml_{nullptr};
};
}  // namespace Widelands

#endif  // end of include guard: WL_GAME_IO_GAME_MAP_PACKET_H
