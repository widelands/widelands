/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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

#ifndef WL_MAP_IO_MAP_PLAYERS_VIEW_PACKET_H
#define WL_MAP_IO_MAP_PLAYERS_VIEW_PACKET_H

#include "map_io/map_data_packet.h"

/// For each player, its view of each node, edge and triangle that he has seen
/// but does not see currently. Information that he currently sees is not
/// loaded but filled in from the game state.
///
/// Also the player's view of the resources that are hidden in the ground.
///
/// This information can not be loaded before the terrains, roads, immovables
/// and players' vision maps are loaded. The vision maps are completely loaded
/// after MapBobdataPacket has been loaded.
MAP_DATA_PACKET(MapPlayersViewPacket)

#endif  // end of include guard: WL_MAP_IO_MAP_PLAYERS_VIEW_PACKET_H
