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

#include "widelands_map_allowed_buildings_data_packet.h"
#include "widelands_map_bob_data_packet.h"
#include "widelands_map_bobdata_data_packet.h"
#include "widelands_map_building_data_packet.h"
#include "widelands_map_buildingdata_data_packet.h"
#include "widelands_map_event_data_packet.h"
#include "widelands_map_flag_data_packet.h"
#include "widelands_map_flagdata_data_packet.h"
#include "widelands_map_data_packet_factory.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_heights_data_packet.h"
#include "widelands_map_immovable_data_packet.h"
#include "widelands_map_immovabledata_data_packet.h"
#include "widelands_map_owned_fields_data_packet.h"
#include "widelands_map_player_names_and_tribes_data_packet.h"
#include "widelands_map_player_position_data_packet.h"
#include "widelands_map_resources_data_packet.h"
#include "widelands_map_road_data_packet.h"
#include "widelands_map_roaddata_data_packet.h"
#include "widelands_map_seen_fields_data_packet.h"
#include "widelands_map_terrain_data_packet.h"
#include "widelands_map_trigger_data_packet.h"
#include "widelands_map_ware_data_packet.h"
#include "widelands_map_waredata_data_packet.h"


Widelands_Map_Data_Packet* Widelands_Map_Data_Packet_Factory::create_correct_packet(ushort id) throw(wexception) {
    switch(id) {
      case PACKET_HEIGHTS: return new Widelands_Map_Heights_Data_Packet(); break;
      case PACKET_TERRAINS: return new Widelands_Map_Terrain_Data_Packet(); break;
      case PACKET_IMMOVABLE: return new Widelands_Map_Immovable_Data_Packet(); break;
      case PACKET_PLAYER_POSITION: return new Widelands_Map_Player_Position_Data_Packet(); break;
      case PACKET_BOB: return new Widelands_Map_Bob_Data_Packet(); break;
      case PACKET_RESOURCES: return new Widelands_Map_Resources_Data_Packet(); break;
      case PACKET_PLAYER_NAM_TRIB: return new Widelands_Map_Player_Names_And_Tribes_Data_Packet(); break;
      case PACKET_TRIGGER: return new Widelands_Map_Trigger_Data_Packet(); break;
      case PACKET_EVENT: return new Widelands_Map_Event_Data_Packet(); break;
      case PACKET_ALLOWED_BUILDINGS: return new Widelands_Map_Allowed_Buildings_Data_Packet(); break;
      case PACKET_FLAG: return new Widelands_Map_Flag_Data_Packet(); break;
      case PACKET_ROAD: return new Widelands_Map_Road_Data_Packet(); break;
      case PACKET_BUILDING: return new Widelands_Map_Building_Data_Packet(); break;
      case PACKET_WARE: return new Widelands_Map_Ware_Data_Packet(); break;
      case PACKET_FLAGDATA: return new Widelands_Map_Flagdata_Data_Packet(); break;
      case PACKET_ROADDATA: return new Widelands_Map_Roaddata_Data_Packet(); break;
      case PACKET_BUILDINGDATA: return new Widelands_Map_Buildingdata_Data_Packet(); break;
      case PACKET_WAREDATA: return new Widelands_Map_Waredata_Data_Packet(); break;
      case PACKET_BOBDATA: return new Widelands_Map_Bobdata_Data_Packet(); break;
      case PACKET_IMMOVABLEDATA: return new Widelands_Map_Immovabledata_Data_Packet(); break;
      case PACKET_OWNED_FIELDS: return new Widelands_Map_Owned_Fields_Data_Packet(); break;
      case PACKET_SEEN_FIELDS: return new Widelands_Map_Seen_Fields_Data_Packet(); break;
      default: throw wexception("Unknown Packet_Data_ID in map file: %i\n", id); break;
    }
    // never here
    return 0;
}

