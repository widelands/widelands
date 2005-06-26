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

#include "editor_game_base.h"
#include "error.h"
#include "filesystem.h"
#include "map.h"
#include "profile.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_player_position_data_packet.h"


#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Player_Position_Data_Packet::~Widelands_Map_Player_Position_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Player_Position_Data_Packet::Read(FileSystem* fs, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader*) throw(wexception) {
   Profile prof;
   prof.read( "player_position", 0, fs );
   Section* s = prof.get_section( "global" );
   
   // read packet version
   int packet_version=s->get_int("packet_version");

   if(packet_version==CURRENT_PACKET_VERSION) {
      // Read all the positions
      // This could bring trouble if one player position
      // is not set (this is possible in the editor), is also
      // -1, -1
      Map* map=egbase->get_map();

      int i;
      char buf[256];
      for(i=1; i<=map->get_nrplayers(); i++) {
         Coords c;
         sprintf( buf, "player_%i_x", i);
         c.x = s->get_int( buf ); 
         sprintf( buf, "player_%i_y", i);
         c.y = s->get_int( buf ); 
         map->set_starting_pos(i,c);
      }
      return;
   }
   assert(0); // never here
}


/*
 * Write Function
 */
void Widelands_Map_Player_Position_Data_Packet::Write(FileSystem* fs, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver*) throw(wexception) {
   Profile prof;
   Section* s = prof.create_section("global");
    
   // packet version
   s->set_int("packet_version", CURRENT_PACKET_VERSION);

   // Now, all positions in order, first x, then y
   Map* map=egbase->get_map();
   int i=0;
   char buf[256];
   for(i=1; i<=map->get_nrplayers(); i++) {
      Coords c=map->get_starting_pos(i);
      sprintf( buf, "player_%i_x", i);
      s->set_int( buf, c.x ); 
      sprintf( buf, "player_%i_y", i);
      s->set_int( buf, c.y ); 
   }

   prof.write("player_position", false, fs );
}
