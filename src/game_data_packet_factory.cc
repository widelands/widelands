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

#include "game_cmd_queue_data_packet.h"
#include "game_computer_player_data_packet.h"
#include "game_data_packet_factory.h"
#include "game_data_packet_ids.h"
#include "game_game_class_data_packet.h"
#include "game_interactive_player_data_packet.h"
#include "game_map_data_packet.h"
#include "game_player_economies_data_packet.h"
#include "game_player_info_data_packet.h"


Game_Data_Packet* Game_Data_Packet_Factory::create_correct_packet(ushort id) throw(wexception) {
    switch(id) {
       case PACKET_GAME_CLASS_DATA: return new Game_Game_Class_Data_Packet(); break;
       case PACKET_PLAYER_INFO_DATA: return new Game_Player_Info_Data_Packet(); break;
       case PACKET_MAP_DATA: return new Game_Map_Data_Packet(); break;
       case PACKET_PLAYER_ECONOMIES_DATA: return new Game_Player_Economies_Data_Packet(); break;
       case PACKET_CMD_QUEUE_DATA: return new Game_Cmd_Queue_Data_Packet(); break;
       case PACKET_INTERACTIVE_PLAYER_DATA: return new Game_Interactive_Player_Data_Packet(); break;
       case PACKET_COMPUTER_PLAYER_DATA: return new Game_Computer_Player_Data_Packet(); break;
       default: throw wexception("Unknown Packet_Data_ID in saved-game file: %i\n", id); break;
    }
    // never here
    return 0;
}
