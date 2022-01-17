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

#ifndef WL_MAP_IO_MAP_PLAYER_NAMES_AND_TRIBES_PACKET_H
#define WL_MAP_IO_MAP_PLAYER_NAMES_AND_TRIBES_PACKET_H

#include "map_io/map_data_packet.h"

namespace Widelands {

class Map;

/*
 * This data packet contains player names
 * and tribes (scenario packet)
 */
struct MapPlayerNamesAndTribesPacket {
	virtual ~MapPlayerNamesAndTribesPacket() = default;

	virtual void read(FileSystem&, EditorGameBase&, bool, MapObjectLoader&);
	void write(FileSystem&, EditorGameBase&, MapObjectSaver&);

	void pre_read(FileSystem&, Map*, bool skip);
};
}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_PLAYER_NAMES_AND_TRIBES_PACKET_H
