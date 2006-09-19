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

#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "game_game_class_data_packet.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Game_Game_Class_Data_Packet::~Game_Game_Class_Data_Packet(void) {
}

/*
 * Read Function
 */
void Game_Game_Class_Data_Packet::Read(FileSystem* fs, Game* game, Widelands_Map_Map_Object_Loader*) throw(_wexception) {
   FileRead fr;

   fr.Open( fs, "binary/game_class" );

   // read packet version
   int packet_version=fr.Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      // Can't load netgames
      game->m_netgame=0;

      game->m_state=gs_running;
      game->m_speed=fr.Signed16();

      game->m_gametime=fr.Unsigned32();

      game->m_conquer_info.resize(fr.Unsigned16());
      for(uint i=0; i<game->m_conquer_info.size(); i++) {
         game->m_conquer_info[i].player = fr.Unsigned8();
         game->m_conquer_info[i].middle_point.x = fr.Unsigned16();
         game->m_conquer_info[i].middle_point.y = fr.Unsigned16();
         game->m_conquer_info[i].area = fr.Unsigned16();
      }
      // DONE
      return;
   } else
      throw wexception("Unknown version in Game_Game_Class_Data_Packet: %i\n", packet_version);

   assert(0); // never here
}

/*
 * Write Function
 */
void Game_Game_Class_Data_Packet::Write(FileSystem* fs, Game* game, Widelands_Map_Map_Object_Saver*) throw(_wexception) {
   FileWrite fw;

   // Packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // Can't save netgames (TODO)
   assert(!game->m_netgame);

   // State is running, we do not need to save this
	// Save speed
   fw.Signed16(game->m_speed);

   // From the interactive player, is saved somewhere else
   // Computer players are saved somewhere else

   // CMD Queue is saved later
   // We do not care for real time.

   // EDITOR GAME CLASS
   // Write gametime
   fw.Unsigned32(game->m_gametime);

   // We do not care for players, since they were set
   // on game initialization to match Map::scenario_player_[names|tribes]
   // or vice versa, so this is handled by map loader

   // Objects are loaded and saved by map

   // Tribes and wares are handled by map
   // Interactive_Base doesn't need saving

   // Map is handled by map saving

   // Track pointers are not saved in save games

   // Conquer info
   fw.Unsigned16(game->m_conquer_info.size());
   for(uint i=0; i<game->m_conquer_info.size(); i++) {
      fw.Unsigned8(game->m_conquer_info[i].player);
      fw.Unsigned16(game->m_conquer_info[i].middle_point.x);
      fw.Unsigned16(game->m_conquer_info[i].middle_point.y);
      fw.Unsigned16(game->m_conquer_info[i].area);
   }

   fw.Write( fs, "binary/game_class" );
}
