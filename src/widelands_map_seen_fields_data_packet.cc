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

#include "widelands_map_seen_fields_data_packet.h"
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include "widelands_map_data_packet_ids.h"
#include "error.h"


#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Seen_Fields_Data_Packet::~Widelands_Map_Seen_Fields_Data_Packet(void) {
}

/*
 * Read Function
 */
void Widelands_Map_Seen_Fields_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase, bool skip, Widelands_Map_Map_Object_Loader*) throw(wexception) {
   // read packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      // Read all the seen_fields
      Map* map=egbase->get_map();

      for(ushort y=0; y<map->get_height(); y++) {
         for(ushort x=0; x<map->get_width(); x++) {
            ushort data=fr->Unsigned16();
            if(!skip) {
               for(uint i=0; i<egbase->get_map()->get_nrplayers(); i++) {
                  Player* plr = egbase->get_player(i+1);
                  if(plr)
                     plr->set_area_seen(Coords(x,y), 0, data & ( 1 << i ));
               }
            }

         }
      }
      return;
   }
   throw wexception("Unknown version in Widelands_Map_Seen_Fields_Data_Packet: %i\n", packet_version);
   assert(0); // never here
}

/*
 * Write Function
 */
void Widelands_Map_Seen_Fields_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase, Widelands_Map_Map_Object_Saver*) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_SEEN_FIELDS);

   // Now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   /*
    * Seen fields are written as followed. The map
    * is passed, for each field it is checked if it 
    * is seen for every player. If it is, the players
    * corresponding bit it set to true. The corresponding
    * bit is  (1 << (PLAYER_NUMBER-1)). This is written 
    * out as Unsigned16 value, so if the allowed player
    * number is sometime bigger than 16, this packet needs
    * reworking
    */
   assert(MAX_PLAYERS < 16);
   Map* map=egbase->get_map();
   for(ushort y=0; y<map->get_height(); y++) {
      for(ushort x=0; x<map->get_width(); x++) {
         ushort data=0;
         for(uint i=0; i<egbase->get_map()->get_nrplayers(); i++) { 
            Player* plr=egbase->get_player(i+1);
            if(plr && plr->is_field_seen(Coords(x,y))) 
               data |= ( 1 << i ); 
         }
         
         fw->Unsigned16(data);
      }
   }
}
