/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands_map_data_packet_factory.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_heights_data_packet.h"
#include "widelands_map_terrain_data_packet.h"
#include "widelands_map_immovable_data_packet.h"
#include "widelands_map_player_position_data_packet.h"
#include "widelands_map_bob_data_packet.h"

Widelands_Map_Data_Packet* Widelands_Map_Data_Packet_Factory::create_correct_packet(ushort id) throw(wexception) {
    switch(id) {
      case PACKET_HEIGHTS: return new Widelands_Map_Heights_Data_Packet(); break;
      case PACKET_TERRAINS: return new Widelands_Map_Terrain_Data_Packet(); break;
      case PACKET_IMMOVABLE: return new Widelands_Map_Immovable_Data_Packet(); break;
      case PACKET_PLAYER_POSITION: return new Widelands_Map_Player_Position_Data_Packet(); break;
      case PACKET_BOB: return new Widelands_Map_Bob_Data_Packet(); break;
      default: throw wexception("Unknown Packet_Data_ID in map file: %i\n", id); break;
    }
    // never here
    return 0;
}

