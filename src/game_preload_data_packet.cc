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

#include "game.h"
#include "game_preload_data_packet.h"

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Game_Preload_Data_Packet::~Game_Preload_Data_Packet(void) {
}

/*
 * Read Function
 */
void Game_Preload_Data_Packet::Read(FileRead* fr, Game* game, Widelands_Map_Map_Object_Loader*) throw(wexception) {
   const char* buf=static_cast<char*>(fr->Data(strlen(WLGF_MAGIC)));
   
   if(strncmp(buf, WLGF_MAGIC, strlen(WLGF_MAGIC))) 
      throw wexception("Not a saved game file!\n");

   // read packet version
   int packet_version=fr->Unsigned16();

   if(packet_version==CURRENT_PACKET_VERSION) {
      m_gametime = fr->Unsigned32();
      m_mapname = fr->CString();

      // DONE
      return;
   } else
      throw wexception("Unknown version in Game_Preload_Data_Packet: %i\n", packet_version);
   assert(0); // never here
}

/*
 * Write Function
 */
void Game_Preload_Data_Packet::Write(FileWrite* fw, Game* game, Widelands_Map_Map_Object_Saver*) throw(wexception) {
   
   // Start with GameFile Magic
   fw->Data(WLGF_MAGIC, strlen(WLGF_MAGIC));
   
   // Now packet version
   fw->Unsigned16(CURRENT_PACKET_VERSION);

   // save some kind of header.
   fw->Unsigned32(game->get_gametime()); // Time in milliseconds of elapsed game time (without pauses)
   fw->CString(game->get_map()->get_name()); // Name of map
}
