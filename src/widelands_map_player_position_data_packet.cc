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

#include "widelands_map_player_position_data_packet.h"
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "widelands_map_data_packet_ids.h"
#include "error.h"


#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Player_Position_Data_Packet::~Widelands_Map_Player_Position_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Player_Position_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader*) throw(wexception) {
   // read packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      // Read all the positions
      // This could bring trouble if one player position
      // is not set (this is possible in the editor), is also
      // -1, -1
      Map* map=egbase->get_map();

      int i;
      for(i=1; i<=map->get_nrplayers(); i++) {
         Coords c;
         c.x=fr->Signed16();
         c.y=fr->Signed16();
         map->set_starting_pos(i,c);
      }
      return;
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Player_Position_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver*) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_PLAYER_POSITION);

   // Now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // Now, all positions in order, first x, then y
   Map* map=egbase->get_map();
   int i=0;
   for(i=1; i<=map->get_nrplayers(); i++) {
      Coords c=map->get_starting_pos(i);
      fw->Signed16(c.x);
      fw->Signed16(c.y);
   }
}
