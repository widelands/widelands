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

#include <map>
#include "filesystem.h"
#include "editor_game_base.h"
#include "map.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_player_names_and_tribes_data_packet.h"
#include "error.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Player_Names_And_Tribes_Data_Packet::~Widelands_Map_Player_Names_And_Tribes_Data_Packet(void) {
}

/*
 * Read Function
 *
 * this is a scenario packet, it might be that we have to skip it
 */
void Widelands_Map_Player_Names_And_Tribes_Data_Packet::Read(FileRead* fr, Editor_Game_Base* egbase) throw(wexception) {
   Pre_Read(fr, egbase->get_map());
}

/*
 * Pre Read function
 */
void Widelands_Map_Player_Names_And_Tribes_Data_Packet::Pre_Read(FileRead* fr, Map* map) {
   // First packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
     std::string name, tribe;
      int i;
     log("Nr Players: %i\n", map->get_nrplayers());
      for(i=1; i<=map->get_nrplayers(); i++) {
         name=fr->CString();
         tribe=fr->CString();
         if(!get_scenario_skip()) {
            log("setting name/tribe of %i to %s/%s\n", i,name.c_str(), tribe.c_str());
            map->set_scenario_player_name(i,name);
            map->set_scenario_player_tribe(i,tribe);
         }
      }
      return;
   }
   throw wexception("Wrong packet version for Player_Names_And_Tribes_Data_Packet: %i\n", packet_version); 
}

/*
 * Write Function
 */
void Widelands_Map_Player_Names_And_Tribes_Data_Packet::Write(FileWrite* fw, Editor_Game_Base* egbase) throw(wexception) {
   // first of all the magic bytes
   fw->Unsigned16(PACKET_PLAYER_NAM_TRIB);

   // now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   int i=0; 
   Map* map=egbase->get_map();
   std::string name, tribe;
   for(i=1; i<=map->get_nrplayers(); i++) {
      tribe=map->get_scenario_player_tribe(i);
      name=map->get_scenario_player_name(i);
      fw->Data(name.c_str(), name.size());
      fw->Unsigned8('\0');
      fw->Data(tribe.c_str(), tribe.size());
      fw->Unsigned8('\0');
   }
   // DONE
}
